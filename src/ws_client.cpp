// ─── ws_client.cpp — connect to remote WS server, receive scripts ───────────
// Ported from famiglia-ink display_js.cpp, adapted for kairos-core IPC.

#include "ws_client.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <WiFi.h>
#include <WebSocketsClient.h>
#include <esp_system.h>
#include <ArduinoJson.h>
#include "config.h"
#include "board.h"
#include "display_utils.h"
#include "config_store.h"
#include "ipc.h"
#include "elk.h"

static WebSocketsClient ws;
static volatile bool wsConnected = false;
static volatile bool wsShouldRun = false;

static String deviceId;

static String getDeviceId() {
    uint64_t mac = ESP.getEfuseMac();
    char buf[13];
    snprintf(buf, sizeof(buf), "%04X%08X",
             (uint16_t)(mac >> 32), (uint32_t)mac);
    return String(buf);
}

// Parse incoming WS message, extract JS code
static String unwrapJsPayload(const uint8_t *payload, size_t length) {
    String source;
    source.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        source += static_cast<char>(payload[i]);
    }
    source.trim();

    // Try JSON format: {"type":"exec","code":"..."}
    if (source.startsWith("{")) {
        JsonDocument doc;
        if (deserializeJson(doc, source) == DeserializationError::Ok) {
            const char *type = doc["type"] | "";
            const char *wrapped = nullptr;

            if (strcmp(type, "exec") == 0 && doc["code"].is<const char*>()) {
                wrapped = doc["code"].as<const char*>();
            } else if (doc["code"].is<const char*>()) {
                wrapped = doc["code"].as<const char*>();
            } else if (doc["js"].is<const char*>()) {
                wrapped = doc["js"].as<const char*>();
            } else if (doc["script"].is<const char*>()) {
                wrapped = doc["script"].as<const char*>();
            }

            if (wrapped) {
                source = wrapped;
                source.trim();
            } else {
                DBG("wsc", "Skipping non-exec JSON (type='%s')", type);
                return String();
            }
        }
    }

    // Strip markdown fences
    if (source.startsWith("```")) {
        int firstNewline = source.indexOf('\n');
        if (firstNewline >= 0) {
            source.remove(0, firstNewline + 1);
        }
        int closingFence = source.lastIndexOf("```");
        if (closingFence >= 0) {
            source.remove(closingFence);
        }
        source.trim();
    }

    return source;
}

// Push script to scheduler queue
static void enqueueScript(const String& code) {
    if (code.length() == 0 || code.length() >= SCRIPT_CODE_MAX) {
        DBG("wsc", "Script too large or empty (%u bytes)", (unsigned)code.length());
        return;
    }

    ScriptJob* job = (ScriptJob*)malloc(sizeof(ScriptJob));
    if (!job) {
        DBG("wsc", "Failed to allocate ScriptJob");
        return;
    }
    strncpy(job->name, "__ws__", SCRIPT_NAME_MAX - 1);
    job->name[SCRIPT_NAME_MAX - 1] = '\0';
    memcpy(job->code, code.c_str(), code.length());
    job->code[code.length()] = '\0';
    job->codeLen = code.length();

    if (xQueueSend(g_scriptQueue, &job, pdMS_TO_TICKS(100)) != pdTRUE) {
        DBG("wsc", "Script queue full — dropping");
        free(job);
    }
}

static void wsEvent(WStype_t type, uint8_t *payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            DBG("wsc", "Disconnected");
            wsConnected = false;
            break;

        case WStype_CONNECTED:
            DBG("wsc", "Connected to %s", (char *)payload);
            wsConnected = true;
            {
                String hello = "{\"type\":\"hello\",\"deviceId\":\"";
                hello += deviceId;
                hello += "\",\"width\":";
                hello += display.width();
                hello += ",\"height\":";
                hello += display.height();
                hello += ",\"elk\":\"";
                hello += JS_VERSION;
                hello += "\"}";
                ws.sendTXT(hello);
            }
            break;

        case WStype_TEXT:
            DBG("wsc", "Received %u bytes", (unsigned)length);
            {
                String source = unwrapJsPayload(payload, length);
                if (source.length() > 0) {
                    DBG("wsc", "Enqueuing %u bytes for execution", (unsigned)source.length());
                    enqueueScript(source);
                }
            }
            break;

        case WStype_PING:
        case WStype_PONG:
            break;

        default:
            break;
    }
}

void wsClientTask(void* param) {
    (void)param;
    wsShouldRun = true;
    deviceId = getDeviceId();

    KairosConfig& cfg = configGet();

    if (!cfg.ws_enabled || cfg.ws_host.length() == 0) {
        DBG("wsc", "WS client disabled or no host configured");
        wsShouldRun = false;
        vTaskDelete(nullptr);
        return;
    }

    // Build path with device_id substitution
    String path = cfg.ws_path;
    path.replace("{device_id}", deviceId);

    DBG("wsc", "Connecting to %s:%d%s (ssl=%d)",
        cfg.ws_host.c_str(), cfg.ws_port, path.c_str(), cfg.ws_ssl);

    if (cfg.ws_ssl) {
        ws.beginSSL(cfg.ws_host.c_str(), cfg.ws_port, path.c_str());
    } else {
        ws.begin(cfg.ws_host.c_str(), cfg.ws_port, path.c_str());
    }

    ws.onEvent(wsEvent);
    ws.setReconnectInterval(5000);

    while (wsShouldRun) {
        ws.loop();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    ws.disconnect();
    wsConnected = false;
    DBG("wsc", "WS client stopped");
    vTaskDelete(nullptr);
}

void wsClientStop() {
    wsShouldRun = false;
}

bool wsClientIsConnected() {
    return wsConnected;
}
