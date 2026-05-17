// ─── main.cpp — Kairos-Core entry point ─────────────────────────────────────
// Dual-core firmware for Paperd.ink Classic (ESP32).
// Core 0: WiFi, HTTP server, WS client
// Core 1: Scheduler + Elk JS engine + Display

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <LittleFS.h>
#include <WiFi.h>
#include <esp_system.h>
#include <esp_heap_caps.h>

#include "config.h"
#include "board.h"
#include "display_utils.h"
#include "ipc.h"
#include "config_store.h"
#include "scheduler.h"
#include "web_api.h"
#include "ws_client.h"
#include "fetch_worker.h"
#include "mqtt_client.h"
#include "mesh.h"
#include "script_store.h"

// ─── Global IPC queues ──────────────────────────────────────────────────────
QueueHandle_t g_scriptQueue   = nullptr;
QueueHandle_t g_fetchReqQueue = nullptr;
QueueHandle_t g_fetchRspQueue = nullptr;
QueueHandle_t g_mqttInQueue   = nullptr;
QueueHandle_t g_mqttOutQueue  = nullptr;

// ─── Task handles ───────────────────────────────────────────────────────────
static TaskHandle_t schedulerTaskHandle   = nullptr;
static TaskHandle_t webApiTaskHandle      = nullptr;
static TaskHandle_t wsClientTaskHandle    = nullptr;
static TaskHandle_t fetchWorkerTaskHandle = nullptr;
static TaskHandle_t mqttClientTaskHandle  = nullptr;

static constexpr const char* kSetupApSsid = "kairos-ink";
static constexpr uint32_t kSetupHoldMs = 2000;

static void displaySetupModeScreen() {
    String ipStr = WiFi.softAPIP().toString();
    String msg = "Setup mode\nWiFi: ";
    msg += kSetupApSsid;
    msg += "\nOpen:\nhttp://";
    msg += ipStr;
    msg += "\n/setup";
    displayText(msg.c_str());
}

static bool shouldEnterSetupModeOnBoot() {
    if (buttonsHeld(Pin::kBtnMenu, Pin::kBtnSelect, kSetupHoldMs)) {
        DBG("boot", "Setup mode forced by Menu+Select hold");
        return true;
    }
    if (configConsumeSetupModeRequest()) {
        DBG("boot", "Setup mode forced by stored flag");
        return true;
    }
    if (!configHasWifiCredentials()) {
        DBG("boot", "No persisted WiFi credentials; entering setup mode");
        return true;
    }
    return false;
}

static bool consumeSetupChordRequest() {
    static uint32_t chordStartMs = 0;
    static bool chordTriggered = false;

    bool bothDown = buttonDown(Pin::kBtnMenu) && buttonDown(Pin::kBtnSelect);
    if (!bothDown) {
        chordStartMs = 0;
        chordTriggered = false;
        return false;
    }

    if (chordStartMs == 0) chordStartMs = millis();
    if (!chordTriggered && millis() - chordStartMs >= kSetupHoldMs) {
        chordTriggered = true;
        return true;
    }
    return false;
}

void setup() {
    Serial.begin(115200);
    delay(100);
    DBG("boot", "Kairos-Core starting...");
    randomSeed(static_cast<uint32_t>(esp_random()));

    // ── Hardware init ───────────────────────────────────────────────────
    initBoard();
    enableDisplayPower();
    display.init(115200);
    logHeap("boot");

    displayText("Kairos booting...");

    // ── Filesystem ──────────────────────────────────────────────────────
    if (!LittleFS.begin(true)) {
        DBG("fs", "LittleFS mount failed");
        displayText("Filesystem error!\nLittleFS mount failed.");
    } else {
        DBG("fs", "LittleFS mounted");
    }

    // ── Load config ─────────────────────────────────────────────────────
    configInit();
    logHeap("post-config");

    bool requestedSetupMode = shouldEnterSetupModeOnBoot();
    bool setupPortalActive = false;

    // ── WiFi ────────────────────────────────────────────────────────────
    if (requestedSetupMode) {
        displayText("Starting setup\nWiFi...");
        setupPortalActive = startSetupAccessPoint(kSetupApSsid);
        if (setupPortalActive) {
            displaySetupModeScreen();
        } else {
            displayText("Setup WiFi failed");
            DBG("wifi", "Setup AP failed");
        }
    } else {
        displayText("Connecting to WiFi...");
        if (!connectWiFi()) {
            DBG("wifi", "WiFi failed — falling back to setup mode");
            displayText("WiFi failed\nStarting setup...");
            setupPortalActive = startSetupAccessPoint(kSetupApSsid);
            if (setupPortalActive) {
                displaySetupModeScreen();
            } else {
                displayText("Setup WiFi failed");
                DBG("wifi", "Setup AP failed after STA timeout");
            }
        } else {
        String ipStr = WiFi.localIP().toString();
        String msg = "WiFi connected\n" + ipStr + "\nhttp://" + ipStr;
        displayText(msg.c_str());
        DBG("boot", "IP: %s", ipStr.c_str());

        // ── NTP time sync ───────────────────────────────────────────────
        KairosConfig& cfgNtp = configGet();
        const char* ntpServer = cfgNtp.ntp_server.length() > 0
                              ? cfgNtp.ntp_server.c_str() : "pool.ntp.org";
        const char* tzInfo    = cfgNtp.tz_info.length() > 0
                              ? cfgNtp.tz_info.c_str() : "EST5EDT,M3.2.0,M11.1.0";
        configTzTime(tzInfo, ntpServer);
        DBG("ntp", "NTP configured: server=%s tz=%s", ntpServer, tzInfo);
        // Wait briefly for NTP sync (non-blocking after this)
        for (int i = 0; i < 20; i++) {
            time_t now = time(nullptr);
            if (now > 1600000000) {
                struct tm ti;
                localtime_r(&now, &ti);
                DBG("ntp", "Time synced: %04d-%02d-%02d %02d:%02d:%02d",
                    ti.tm_year + 1900, ti.tm_mon + 1, ti.tm_mday,
                    ti.tm_hour, ti.tm_min, ti.tm_sec);
                break;
            }
            delay(250);
        }

        delay(2000);  // show IP for 2 seconds
        }
    }
    logHeap("post-wifi");

    // ── ESP-NOW mesh discovery ──────────────────────────────────────────
    if (!setupPortalActive && WiFi.status() == WL_CONNECTED && configHasWifiCredentials()) {
        meshInit();
    }

    // ── Create IPC queues (pointer queues — each element is a pointer) ────
    g_scriptQueue   = xQueueCreate(SCRIPT_QUEUE_DEPTH,   sizeof(ScriptJob*));
    g_fetchReqQueue = xQueueCreate(FETCH_QUEUE_DEPTH,    sizeof(FetchRequest*));
    g_fetchRspQueue = xQueueCreate(FETCH_QUEUE_DEPTH,    sizeof(FetchResponse*));
    g_mqttInQueue   = xQueueCreate(MQTT_IN_QUEUE_DEPTH,  sizeof(MqttMessage*));
    g_mqttOutQueue  = xQueueCreate(MQTT_OUT_QUEUE_DEPTH, sizeof(MqttCommand*));

    if (!g_scriptQueue || !g_fetchReqQueue || !g_fetchRspQueue
        || !g_mqttInQueue || !g_mqttOutQueue) {
        DBG("ipc", "Failed to create IPC queues!");
        displayText("FATAL: Queue creation\nfailed!");
        return;
    }
    DBG("ipc", "IPC queues created");

    // ── Start scheduler on Core 1 ───────────────────────────────────────
    BaseType_t rc = xTaskCreatePinnedToCore(
        schedulerTask, "scheduler", 16384, nullptr, 2,
        &schedulerTaskHandle, 1  // Core 1
    );
    if (rc != pdPASS) {
        DBG("boot", "Failed to start scheduler task!");
        displayText("FATAL: Scheduler\ntask failed!");
        return;
    }

    // ── Start web API on Core 0 ─────────────────────────────────────────
    webApiSetSetupMode(setupPortalActive);
    if (setupPortalActive || WiFi.status() == WL_CONNECTED) {
        rc = xTaskCreatePinnedToCore(
            webApiTask, "httpd", 16384, nullptr, 1,
            &webApiTaskHandle, 0  // Core 0
        );
        if (rc != pdPASS) {
            DBG("boot", "Failed to start web API task!");
        }

        if (!setupPortalActive && WiFi.status() == WL_CONNECTED) {
            // ── Start WS client on Core 0 (if configured) ──────────────
            KairosConfig& cfg = configGet();
            if (cfg.ws_enabled && cfg.ws_host.length() > 0) {
                rc = xTaskCreatePinnedToCore(
                    wsClientTask, "ws_client", 8192, nullptr, 2,
                    &wsClientTaskHandle, 0  // Core 0
                );
                if (rc != pdPASS) {
                    DBG("boot", "Failed to start WS client task!");
                }
            }

            // ── Start fetch worker on Core 0 (always, for http.get/post) ─
            rc = xTaskCreatePinnedToCore(
                fetchWorkerTask, "fetch", 12288, nullptr, 1,
                &fetchWorkerTaskHandle, 0  // Core 0
            );
            if (rc != pdPASS) {
                DBG("boot", "Failed to start fetch worker!");
            }

            // ── Start MQTT client on Core 0 (if configured) ────────────
            if (cfg.mqtt_enabled && cfg.mqtt_broker.length() > 0) {
                rc = xTaskCreatePinnedToCore(
                    mqttClientTask, "mqtt", 8192, nullptr, 1,
                    &mqttClientTaskHandle, 0  // Core 0
                );
                if (rc != pdPASS) {
                    DBG("boot", "Failed to start MQTT client task!");
                }
            }
        }
    }

    // ── Ready screen ────────────────────────────────────────────────────
    if (setupPortalActive) {
        displaySetupModeScreen();
    } else if (WiFi.status() == WL_CONNECTED) {
        String readyMsg = "Kairos ready\n" + WiFi.localIP().toString()
                        + "\n" + configGet().device_name;
        displayText(readyMsg.c_str());
    } else {
        displayText("Kairos ready\n(no WiFi)");
    }

    logHeap("boot-complete");
    DBG("boot", "Setup complete — scheduler on Core 1, HTTP on Core 0");
}

// ─── Display mesh status on e-paper ─────────────────────────────────────────
static void displayMeshStatus() {
    if (!meshIsReady()) {
        displayText("Mesh unavailable\nConnect to WiFi first");
        DBG("ui", "Skipped mesh status; mesh not ready");
        return;
    }

    KairosConfig& cfg = configGet();
    String lines = cfg.device_name + "\n"
                 + WiFi.localIP().toString() + "\n"
                 + "───────────────\n";

    MeshPeer peers[MESH_MAX_PEERS];
    int n = meshGetPeers(peers, MESH_MAX_PEERS);

    if (n == 0) {
        lines += "No peers found";
    } else {
        lines += String(n) + " peer(s):\n";
        for (int i = 0; i < n; i++) {
            IPAddress ip(peers[i].ip);
            lines += String(peers[i].name) + "\n" + ip.toString() + "\n";
        }
    }

    displayText(lines.c_str());
    DBG("ui", "Displayed mesh status (%d peers)", n);
}

void loop() {
    // Main loop runs on Core 1 by default in Arduino.
    // Handles buttons and periodic mesh status display.

    if (webApiConsumeRestartRequest()) {
        displayText("Setup saved\nRestarting...");
        delay(800);
        ESP.restart();
    }

    bool setupChordActive = buttonDown(Pin::kBtnMenu) && buttonDown(Pin::kBtnSelect);
    if (consumeSetupChordRequest()) {
        if (configRequestSetupMode()) {
            DBG("btn", "Menu+Select held — restarting into setup mode");
            displayText("Setup mode\nRestarting...");
            delay(800);
            ESP.restart();
        } else {
            DBG("btn", "Failed to persist setup mode request");
            displayText("Setup request\nfailed");
            delay(1200);
        }
    }
    if (setupChordActive) {
        vTaskDelay(20 / portTICK_PERIOD_MS);
        return;
    }

    // ── Top button (Menu/GPIO14): restart ────────────────────────────────
    if (buttonPressed(Pin::kBtnMenu)) {
        DBG("btn", "Menu pressed — restarting");
        displayText("Restarting...");
        delay(500);
        ESP.restart();
    }

    // ── Second button (Next/GPIO27): show mesh status ────────────────────
    if (buttonPressed(Pin::kBtnNext)) {
        DBG("btn", "Next pressed — showing mesh status");
        displayMeshStatus();
    }

    // ── Third button (Select/GPIO4): show device info ────────────────────
    if (buttonPressed(Pin::kBtnSelect)) {
        DBG("btn", "Select pressed — showing device info");
        KairosConfig& cfg = configGet();
        String info = cfg.device_name + "\n"
                    + WiFi.localIP().toString() + "\n"
                    + "Heap: " + String(heap_caps_get_free_size(MALLOC_CAP_8BIT)) + "\n"
                    + "Up: " + String(millis() / 1000) + "s\n"
                    + "RSSI: " + String(WiFi.RSSI()) + " dBm";
        displayText(info.c_str());
    }

    // ── Bottom button (Weather/GPIO2): run startup script if saved ───────
    if (buttonPressed(Pin::kBtnWeather)) {
        DBG("btn", "Weather pressed — running 'startup' script");
        String code = scriptLoad("startup");
        if (code.length() > 0 && code.length() < SCRIPT_CODE_MAX) {
            ScriptJob* job = (ScriptJob*)malloc(sizeof(ScriptJob));
            if (job) {
                strncpy(job->name, "startup", SCRIPT_NAME_MAX - 1);
                memcpy(job->code, code.c_str(), code.length());
                job->code[code.length()] = '\0';
                job->codeLen = code.length();
                if (xQueueSend(g_scriptQueue, &job, pdMS_TO_TICKS(500)) != pdTRUE) {
                    free(job);
                }
            }
        } else {
            displayText("No 'startup'\nscript saved");
        }
    }

    // ── Auto-display mesh changes ────────────────────────────────────────
    if (meshIsReady() && meshPeersChanged()) {
        displayMeshStatus();
    }

    vTaskDelay(100 / portTICK_PERIOD_MS);
}
