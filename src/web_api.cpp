// ─── web_api.cpp — HTTP server + routes (Core 0 task) ───────────────────────
// Ported from famiglia-ink web_server.cpp, adapted for kairos-core.
// Scripts are executed via IPC queue to Core 1 scheduler.

#include "web_api.h"
#include "scheduler.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <time.h>
#include <esp_heap_caps.h>
#include "config.h"
#include "board.h"
#include "display_utils.h"
#include "script_store.h"
#include "elk_engine.h"
#include "config_store.h"
#include "ws_client.h"
#include "ipc.h"
#include "ui/index.html.h"
#include "ui/favicon_ico.h"
#include "ui/setup.html.h"
#include "ui/style.css.h"
#include "ui/app.js.h"
#include "mesh.h"
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <vector>

static WebServer server(HTTP_PORT);
static volatile bool taskShouldRun = false;
static volatile bool taskRunning = false;
static volatile bool setupModeActive = false;
static volatile bool restartRequested = false;

// ─── GET / — serve either setup page or the editor ─────────────────────────
static void handleRootPage() {
    if (setupModeActive) {
        server.send_P(200, "text/html", SETUP_HTML);
        return;
    }
    server.send_P(200, "text/html", INDEX_HTML);
}

// ─── GET / — serve the editor ───────────────────────────────────────────────
static void handleEditorPage() {
    server.send_P(200, "text/html", INDEX_HTML);
}

static void handleSetupPage() {
    server.send_P(200, "text/html", SETUP_HTML);
}

static void handleCss() {
    server.send_P(200, "text/css", STYLE_CSS);
}

static void handleJs() {
    server.send_P(200, "application/javascript", APP_JS);
}

static void handleFavicon() {
    server.send_P(200, "image/x-icon", reinterpret_cast<PGM_P>(FAVICON_ICO), sizeof(FAVICON_ICO));
}

// ─── GET /api/scripts — list all stored scripts ─────────────────────────────
static void handleScriptsList() {
    server.send(200, "application/json", scriptListJson());
}

// ─── POST /api/scripts — save a script ──────────────────────────────────────
static void handleScriptSave() {
    if (server.method() != HTTP_POST) {
        server.send(405, "application/json", "{\"error\":\"Method not allowed\"}");
        return;
    }
    JsonDocument doc;
    if (deserializeJson(doc, server.arg("plain"))) {
        server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
    }
    const char* name = doc["name"];
    const char* code = doc["code"];
    if (!name || !code) {
        server.send(400, "application/json", "{\"error\":\"Need 'name' and 'code'\"}");
        return;
    }
    const char* type = doc["type"] | "js";
    const char* cron = doc["cron"] | "";
    if (scriptSave(String(name), String(code), String(type), String(cron))) {
        if (cron && strlen(cron) > 0) {
            DBG("api", "Saved '%s' with cron: [%s]", name, cron);
            // Show what time the cron would next trigger
            time_t now = time(nullptr);
            struct tm ti;
            localtime_r(&now, &ti);
            if (ti.tm_year > 120) {
                DBG("api", "  Device time now: %02d:%02d (wday=%d, mday=%d, mon=%d)",
                    ti.tm_hour, ti.tm_min, ti.tm_wday, ti.tm_mday, ti.tm_mon + 1);
            } else {
                DBG("api", "  NTP not synced — cron will not trigger until time is set");
            }
        } else {
            DBG("api", "Saved '%s' (no cron schedule)", name);
        }
        server.send(200, "application/json", "{\"status\":\"ok\"}");
    } else {
        server.send(400, "application/json", "{\"error\":\"Save failed\"}");
    }
}

// ─── GET /api/scripts/{name} — load a script ───────────────────────────────
static void handleScriptLoad() {
    String uri = server.uri();
    String name = uri.substring(String("/api/scripts/").length());
    if (name.endsWith("/run")) return;

    String scriptType;
    String code = scriptLoad(name, &scriptType);
    if (code.length() == 0) {
        server.send(404, "application/json", "{\"error\":\"Not found\"}");
        return;
    }
    JsonDocument doc;
    doc["code"] = code;
    
    doc["name"] = name;
    doc["type"] = scriptType;
    String metaStr = scriptLoadMeta(name);
    if (metaStr.length() > 0) {
        JsonDocument metaDoc;
        if (deserializeJson(metaDoc, metaStr) == DeserializationError::Ok) {
            if (metaDoc["cron"].is<const char*>()) {
                doc["cron"] = metaDoc["cron"].as<String>();
            }
        }
    }
    String out;

    serializeJson(doc, out);
    server.send(200, "application/json", out);
}

// ─── DELETE /api/scripts/{name} — delete a script ───────────────────────────
static void handleScriptDelete() {
    String uri = server.uri();
    String name = uri.substring(String("/api/scripts/").length());
    if (scriptDelete(name)) {
        server.send(200, "application/json", "{\"status\":\"ok\"}");
    } else {
        server.send(404, "application/json", "{\"error\":\"Not found\"}");
    }
}

// ─── POST /api/scripts/{name}/run — run a stored script ─────────────────────
static void handleScriptRun() {
    String uri = server.uri();
    String name = uri.substring(String("/api/scripts/").length());
    name = name.substring(0, name.length() - 4); // strip "/run"

    String code = scriptLoad(name);
    if (code.length() == 0) {
        server.send(404, "application/json", "{\"error\":\"Not found\"}");
        return;
    }

    if (enqueueCode(name.c_str(), code.c_str(), code.length())) {
        server.send(200, "application/json", "{\"status\":\"ok\"}");
    } else {
        server.send(500, "application/json", "{\"error\":\"Queue full\"}");
    }
}

// ─── POST /api/exec — execute arbitrary code ────────────────────────────────
static void handleScriptExec() {
    if (server.method() != HTTP_POST) {
        server.send(405, "application/json", "{\"error\":\"Method not allowed\"}");
        return;
    }
    JsonDocument doc;
    if (deserializeJson(doc, server.arg("plain"))) {
        server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
    }
    const char* code = doc["code"];
    if (!code) {
        server.send(400, "application/json", "{\"error\":\"Need 'code'\"}");
        return;
    }
    DBG("api", "exec: %u bytes", (unsigned)strlen(code));

    if (enqueueCode("__exec__", code, strlen(code))) {
        server.send(200, "application/json", "{\"status\":\"ok\"}");
    } else {
        server.send(500, "application/json", "{\"error\":\"Queue full or code too large\"}");
    }
}

// ─── POST /api/display/text — show text on display ──────────────────────────
static void handleDisplayText() {
    if (server.method() != HTTP_POST) {
        server.send(405, "application/json", "{\"error\":\"Method not allowed\"}");
        return;
    }
    JsonDocument doc;
    if (deserializeJson(doc, server.arg("plain"))) {
        server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
    }
    const char* text = doc["text"];
    if (!text) {
        server.send(400, "application/json", "{\"error\":\"Need 'text'\"}");
        return;
    }
    // Build a JS script that displays the text via Elk
    // This goes through the scheduler on Core 1 to be safe with display
    String jsCode = "display.beginScene();display.useBodyFont();display.setCursor(10,40);display.print(\"";
    // Simple escape for the text
    String escaped(text);
    escaped.replace("\\", "\\\\");
    escaped.replace("\"", "\\\"");
    jsCode += escaped;
    jsCode += "\");display.update();";

    if (enqueueCode("__text__", jsCode.c_str(), jsCode.length())) {
        server.send(200, "application/json", "{\"status\":\"ok\"}");
    } else {
        server.send(500, "application/json", "{\"error\":\"Queue full\"}");
    }
}

// ─── GET /api/config — get current config ───────────────────────────────────
static void handleConfigGet() {
    server.send(200, "application/json", configToJson(true));
}

// ─── POST /api/config — merge config ────────────────────────────────────────
static void handleConfigSet() {
    if (server.method() != HTTP_POST) {
        server.send(405, "application/json", "{\"error\":\"Method not allowed\"}");
        return;
    }
    String body = server.arg("plain");
    if (configMergeJson(body)) {
        server.send(200, "application/json", "{\"status\":\"ok\"}");
    } else {
        server.send(400, "application/json", "{\"error\":\"Invalid config\"}");
    }
}

static void appendMacAddress(JsonObject obj, const uint8_t* mac) {
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    obj["mac"] = macStr;
}

static void appendLocalStatus(JsonObject obj) {
    String ip = (WiFi.status() == WL_CONNECTED) ? WiFi.localIP().toString()
                                                : WiFi.softAPIP().toString();
    obj["name"] = configGet().device_name;
    obj["device_name"] = configGet().device_name;
    obj["self"] = true;
    obj["ip"] = ip;
    obj["heap_free"] = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    obj["uptime_ms"] = millis();
    obj["ws_connected"] = wsClientIsConnected();
    obj["wifi_rssi"] = WiFi.RSSI();
    obj["mesh_peers"] = meshPeerCount();
    obj["setup_mode"] = setupModeActive;

    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    appendMacAddress(obj, mac);

    time_t now = time(nullptr);
    struct tm ti;
    localtime_r(&now, &ti);
    if (ti.tm_year > 120) {
        char timeBuf[20];
        strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", &ti);
        obj["time"] = timeBuf;
        char dateBuf[12];
        strftime(dateBuf, sizeof(dateBuf), "%Y-%m-%d", &ti);
        obj["date"] = dateBuf;
        obj["epoch"] = (unsigned long)now;
    }
}

static void appendPeerIdentity(JsonObject obj, const MeshPeer& peer) {
    obj["name"] = peer.name;
    IPAddress ip(peer.ip);
    obj["ip"] = ip.toString();
    appendMacAddress(obj, peer.mac);
    obj["self"] = false;
    obj["age_ms"] = millis() - peer.lastSeen;
}

static void mergePeerStatus(JsonObject obj, JsonObjectConst status) {
    if (status["name"].is<const char*>()) obj["name"] = status["name"].as<const char*>();
    if (status["device_name"].is<const char*>()) obj["device_name"] = status["device_name"].as<const char*>();
    if (status["ip"].is<const char*>()) obj["ip"] = status["ip"].as<const char*>();
    if (status["heap_free"].is<unsigned long>() || status["heap_free"].is<int>()) obj["heap_free"] = status["heap_free"];
    if (status["uptime_ms"].is<unsigned long>() || status["uptime_ms"].is<int>()) obj["uptime_ms"] = status["uptime_ms"];
    if (status["ws_connected"].is<bool>()) obj["ws_connected"] = status["ws_connected"];
    if (status["wifi_rssi"].is<int>()) obj["wifi_rssi"] = status["wifi_rssi"];
    if (status["mesh_peers"].is<int>()) obj["mesh_peers"] = status["mesh_peers"];
    if (status["setup_mode"].is<bool>()) obj["setup_mode"] = status["setup_mode"];
    if (status["time"].is<const char*>()) obj["time"] = status["time"].as<const char*>();
    if (status["date"].is<const char*>()) obj["date"] = status["date"].as<const char*>();
    if (status["epoch"].is<unsigned long>() || status["epoch"].is<int>()) obj["epoch"] = status["epoch"];
    if (status["mac"].is<const char*>()) obj["mac"] = status["mac"].as<const char*>();
}

static void appendPeerStatuses(JsonArray peersArray) {
    MeshPeer peers[MESH_MAX_PEERS];
    int peerCount = meshGetPeers(peers, MESH_MAX_PEERS);

    for (int index = 0; index < peerCount; ++index) {
        JsonObject peerObj = peersArray.add<JsonObject>();
        appendPeerIdentity(peerObj, peers[index]);

        HTTPClient http;
        WiFiClient client;
        String url = "http://" + peerObj["ip"].as<String>() + "/api/status?include_peers=0";
        http.begin(client, url);
        http.setTimeout(2500);

        int httpCode = http.GET();
        if (httpCode == 200) {
            JsonDocument peerStatusDoc;
            DeserializationError err = deserializeJson(peerStatusDoc, http.getStream());
            if (!err && peerStatusDoc.is<JsonObject>()) {
                mergePeerStatus(peerObj, peerStatusDoc.as<JsonObjectConst>());
            } else {
                peerObj["status_error"] = "invalid_json";
            }
        } else {
            peerObj["status_error"] = httpCode;
        }
        http.end();
    }
}

// ─── POST /api/setup — save setup config and request restart ───────────────
static void handleSetupSave() {
    if (server.method() != HTTP_POST) {
        server.send(405, "application/json", "{\"error\":\"Method not allowed\"}");
        return;
    }

    JsonDocument doc;
    if (deserializeJson(doc, server.arg("plain"))) {
        server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
    }

    String wifiSsid = doc["wifi_ssid"].as<String>();
    wifiSsid.trim();
    if (wifiSsid.length() == 0) {
        server.send(400, "application/json", "{\"error\":\"WiFi SSID is required\"}");
        return;
    }

    KairosConfig& cfg = configGet();
    cfg.wifi_ssid = wifiSsid;
    if (doc["wifi_pass"].is<const char*>()) cfg.wifi_pass = doc["wifi_pass"].as<String>();
    if (doc["device_name"].is<const char*>()) {
        String deviceName = doc["device_name"].as<String>();
        deviceName.trim();
        if (deviceName.length() > 0) cfg.device_name = deviceName;
    }
    if (doc["ntp_server"].is<const char*>()) cfg.ntp_server = doc["ntp_server"].as<String>();
    if (doc["tz_info"].is<const char*>()) cfg.tz_info = doc["tz_info"].as<String>();

    if (!configSave()) {
        server.send(500, "application/json", "{\"error\":\"Failed to save config\"}");
        return;
    }

    restartRequested = true;
    DBG("api", "Setup saved; restart requested");
    server.send(200, "application/json", "{\"status\":\"ok\",\"restart\":true}");
}

// ─── GET /api/status — device status ────────────────────────────────────────
static void handleStatus() {
    JsonDocument doc;
    JsonObject root = doc.to<JsonObject>();
    appendLocalStatus(root);

    bool includePeers = true;
    if (server.hasArg("include_peers")) {
        String includePeersArg = server.arg("include_peers");
        includePeers = !(includePeersArg == "0" || includePeersArg == "false");
    }

    if (includePeers) {
        JsonArray peers = root["peers"].to<JsonArray>();
        appendPeerStatuses(peers);
    }

    String out;
    serializeJson(doc, out);
    server.send(200, "application/json", out);
}

// ─── GET /api/mesh/peers — list all known mesh peers ────────────────────────
static void handleMeshPeers() {
    server.send(200, "application/json", meshPeersJson());
}

// ─── POST /api/mesh/exec — forward script to one or more mesh peers ────────
static void handleMeshExec() {
    if (server.method() != HTTP_POST) {
        server.send(405, "application/json", "{\"error\":\"Method not allowed\"}");
        return;
    }
    JsonDocument doc;
    if (deserializeJson(doc, server.arg("plain"))) {
        server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
    }
    const char* code = doc["code"];
    if (!code) {
        server.send(400, "application/json", "{\"error\":\"Need 'code'\"}");
        return;
    }

    // Targets: array of IP strings, or "all"
    JsonDocument result;
    bool runLocal = false;
    String myIp = WiFi.localIP().toString();

    // Collect target IPs
    std::vector<String> targets;
    if (doc["targets"].is<JsonArray>()) {
        for (JsonVariant t : doc["targets"].as<JsonArray>()) {
            String ip = t.as<String>();
            if (ip == myIp) {
                runLocal = true;
            } else {
                targets.push_back(ip);
            }
        }
    } else {
        // No targets specified = run on all peers + local
        runLocal = true;
        MeshPeer peers[MESH_MAX_PEERS];
        int n = meshGetPeers(peers, MESH_MAX_PEERS);
        for (int i = 0; i < n; i++) {
            IPAddress ip(peers[i].ip);
            targets.push_back(ip.toString());
        }
    }

    // Run locally if needed
    if (runLocal) {
        bool ok = enqueueCode("__mesh_exec__", code, strlen(code));
        JsonObject r = result["results"].to<JsonObject>()[myIp].to<JsonObject>();
        r["status"] = ok ? "ok" : "queue_full";
        r["name"] = configGet().device_name;
    }

    // Forward to remote peers via HTTP POST
    for (const String& ip : targets) {
        HTTPClient http;
        WiFiClient client;
        String url = "http://" + ip + "/api/exec";
        http.begin(client, url);
        http.addHeader("Content-Type", "application/json");
        http.setTimeout(5000);

        String body = "{\"code\":";
        // Properly serialize the code string
        JsonDocument codeDoc;
        codeDoc["code"] = code;
        body = "";
        serializeJson(codeDoc, body);

        int httpCode = http.POST(body);
        JsonObject r = result["results"].to<JsonObject>()[ip].to<JsonObject>();
        if (httpCode == 200) {
            r["status"] = "ok";
        } else {
            r["status"] = "error";
            r["http_code"] = httpCode;
        }
        http.end();
    }

    String out;
    serializeJson(result, out);
    server.send(200, "application/json", out);
}

// ─── POST /api/mesh/scripts — save script to one or more mesh peers ───────
static void handleMeshScripts() {
    if (server.method() != HTTP_POST) {
        server.send(405, "application/json", "{\"error\":\"Method not allowed\"}");
        return;
    }
    JsonDocument doc;
    if (deserializeJson(doc, server.arg("plain"))) {
        server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
    }
    const char* name = doc["name"];
    const char* code = doc["code"];
    if (!name || !code) {
        server.send(400, "application/json", "{\"error\":\"Need 'name' and 'code'\"}");
        return;
    }
    const char* type = doc["type"] | "js";
    const char* cron = doc["cron"] | "";

    JsonDocument result;
    bool runLocal = false;
    String myIp = WiFi.localIP().toString();

    std::vector<String> targets;
    if (doc["targets"].is<JsonArray>()) {
        for (JsonVariant t : doc["targets"].as<JsonArray>()) {
            String ip = t.as<String>();
            if (ip == myIp) {
                runLocal = true;
            } else {
                targets.push_back(ip);
            }
        }
    } else {
        runLocal = true;
        MeshPeer peers[MESH_MAX_PEERS];
        int n = meshGetPeers(peers, MESH_MAX_PEERS);
        for (int i = 0; i < n; i++) {
            IPAddress ip(peers[i].ip);
            targets.push_back(ip.toString());
        }
    }

    if (runLocal) {
        JsonObject r = result["results"].to<JsonObject>()[myIp].to<JsonObject>();
        if (scriptSave(String(name), String(code), String(type), String(cron))) {
            r["status"] = "ok";
        } else {
            r["status"] = "error";
        }
        r["name"] = configGet().device_name;
    }

    for (const String& ip : targets) {
        HTTPClient http;
        WiFiClient client;
        String url = "http://" + ip + "/api/scripts";
        http.begin(client, url);
        http.addHeader("Content-Type", "application/json");
        http.setTimeout(5000);

        JsonDocument reqDoc;
        reqDoc["name"] = name;
        reqDoc["code"] = code;
        reqDoc["type"] = type;
        reqDoc["cron"] = cron;
        String body;
        serializeJson(reqDoc, body);

        int httpCode = http.POST(body);
        JsonObject r = result["results"].to<JsonObject>()[ip].to<JsonObject>();
        if (httpCode == 200) {
            r["status"] = "ok";
        } else {
            r["status"] = "error";
            r["http_code"] = httpCode;
        }
        http.end();
    }

    String out;
    serializeJson(result, out);
    server.send(200, "application/json", out);
}

// ─── GET /api/elk/api — return this device's Elk JS API as JSON ─────────────
static void handleElkApi() {
    // Return a structured description of all Elk JS bindings
    static const char API_JSON[] PROGMEM = R"json({
  "device": true,
  "objects": {
    "display": {
      "methods": {
        "beginScene": {"args": [], "desc": "Clear buffer, start new frame"},
        "fillScreen": {"args": ["color"], "desc": "Fill screen with color"},
        "drawPixel": {"args": ["x","y","c"], "desc": "Draw single pixel"},
        "drawLine": {"args": ["x1","y1","x2","y2","c"], "desc": "Draw line"},
        "drawRect": {"args": ["x","y","w","h","c"], "desc": "Draw rectangle outline"},
        "fillRect": {"args": ["x","y","w","h","c"], "desc": "Fill rectangle"},
        "drawCircle": {"args": ["x","y","r","c"], "desc": "Draw circle outline"},
        "fillCircle": {"args": ["x","y","r","c"], "desc": "Fill circle"},
        "drawRoundRect": {"args": ["x","y","w","h","r","c"], "desc": "Rounded rect outline"},
        "fillRoundRect": {"args": ["x","y","w","h","r","c"], "desc": "Fill rounded rect"},
        "drawTriangle": {"args": ["x1","y1","x2","y2","x3","y3","c"], "desc": "Triangle outline"},
        "fillTriangle": {"args": ["x1","y1","x2","y2","x3","y3","c"], "desc": "Fill triangle"},
        "setTextColor": {"args": ["c"], "desc": "Set text color"},
        "setCursor": {"args": ["x","y"], "desc": "Set text cursor"},
        "print": {"args": ["value"], "desc": "Print any value at cursor"},
        "setRotation": {"args": ["r"], "desc": "Set rotation (0-3)"},
        "update": {"args": [], "desc": "Push buffer to e-paper"},
        "width": {"args": [], "ret": "number", "desc": "Display width"},
        "height": {"args": [], "ret": "number", "desc": "Display height"},
        "useBodyFont": {"args": [], "desc": "Switch to body font"},
        "useTitleFont": {"args": [], "desc": "Switch to title font"}
      }
    },
    "http": {
      "methods": {
        "get": {"args": ["url"], "ret": "{status,body}", "desc": "HTTP GET, blocks until response"},
        "post": {"args": ["url","contentType","body"], "ret": "{status,body}", "desc": "HTTP POST"}
      }
    },
    "sys": {
      "methods": {
        "millis": {"args": [], "ret": "number", "desc": "Uptime in ms"},
        "heapFree": {"args": [], "ret": "number", "desc": "Free heap bytes"},
        "deepSleep": {"args": ["seconds"], "desc": "Enter deep sleep"}
      }
    },
    "Math": {
      "methods": {
        "abs": {"args": ["x"]}, "ceil": {"args": ["x"]}, "floor": {"args": ["x"]},
        "round": {"args": ["x"]}, "trunc": {"args": ["x"]}, "sign": {"args": ["x"]},
        "sin": {"args": ["x"]}, "cos": {"args": ["x"]}, "tan": {"args": ["x"]},
        "asin": {"args": ["x"]}, "acos": {"args": ["x"]}, "atan": {"args": ["x"]},
        "atan2": {"args": ["y","x"]}, "sqrt": {"args": ["x"]}, "pow": {"args": ["x","y"]},
        "exp": {"args": ["x"]}, "log": {"args": ["x"]}, "log10": {"args": ["x"]},
        "min": {"args": ["...nums"]}, "max": {"args": ["...nums"]}, "random": {"args": []}
      },
      "constants": ["PI","E","LN2","LN10","LOG2E","LOG10E","SQRT2","SQRT1_2"]
    },
    "JSON": {
      "methods": {
        "stringify": {"args": ["value"], "ret": "string", "desc": "Convert to JSON string"},
        "parse": {"args": ["str"], "ret": "value", "desc": "Parse JSON (no arrays)"}
      }
    },
    "String": {
      "methods": {
        "indexOf": {"args": ["str","search"], "ret": "number", "desc": "Find substring index (-1 if not found)"},
        "substring": {"args": ["str","start","end?"], "ret": "string", "desc": "Extract substring"},
        "length": {"args": ["str"], "ret": "number", "desc": "String length"},
        "replace": {"args": ["str","search","replace"], "ret": "string", "desc": "Replace first occurrence"},
        "from": {"args": ["value"], "ret": "string", "desc": "Convert any value to string"}
      }
    },
    "Date": {
      "methods": {
        "now": {"args": [], "ret": "number", "desc": "Current epoch seconds"},
        "year": {"args": [], "ret": "number", "desc": "Current year (e.g. 2026)"},
        "month": {"args": [], "ret": "number", "desc": "Month 1-12"},
        "day": {"args": [], "ret": "number", "desc": "Day of month 1-31"},
        "hours": {"args": [], "ret": "number", "desc": "Hour 0-23"},
        "minutes": {"args": [], "ret": "number", "desc": "Minute 0-59"},
        "seconds": {"args": [], "ret": "number", "desc": "Second 0-59"},
        "weekday": {"args": [], "ret": "number", "desc": "Day of week 0=Sun..6=Sat"},
        "format": {"args": ["fmt"], "ret": "string", "desc": "strftime format string"}
      }
    }
  },
  "globals": {
    "log": {"args": ["value"], "desc": "Print to serial log"},
    "delay": {"args": ["ms"], "desc": "Blocking delay (max 30s)"},
    "BLACK": {"value": 0},
    "WHITE": {"value": 1}
  }
})json";

    server.send_P(200, "application/json", API_JSON);
}

// ─── GET /api/mesh/api?ip=x.x.x.x — proxy fetch remote device's API ───────
static void handleMeshApi() {
    String ip = server.arg("ip");
    if (ip.length() == 0) {
        server.send(400, "application/json", "{\"error\":\"Need ?ip= parameter\"}");
        return;
    }

    HTTPClient http;
    WiFiClient client;
    String url = "http://" + ip + "/api/elk/api";
    http.begin(client, url);
    http.setTimeout(5000);

    int httpCode = http.GET();
    if (httpCode == 200) {
        String payload = http.getString();
        server.send(200, "application/json", payload);
    } else {
        String err = "{\"error\":\"Remote returned " + String(httpCode) + "\"}";
        server.send(502, "application/json", err);
    }
    http.end();
}

// ─── Route registration ─────────────────────────────────────────────────────
static bool routesRegistered = false;

static void registerRoutes() {
    if (routesRegistered) return;

    server.on("/", handleRootPage);
    server.on("/editor", handleEditorPage);
    server.on("/setup", handleSetupPage);
    server.on("/favicon.ico", handleFavicon);
    server.on("/ui/style.css", handleCss);
    server.on("/ui/app.js", handleJs);

    server.on("/api/scripts", HTTP_GET, handleScriptsList);
    server.on("/api/scripts", HTTP_POST, handleScriptSave);
    server.on("/api/exec", HTTP_POST, handleScriptExec);

    server.on("/api/display/text", HTTP_POST, handleDisplayText);

    server.on("/api/config", HTTP_GET, handleConfigGet);
    server.on("/api/config", HTTP_POST, handleConfigSet);
    server.on("/api/setup", HTTP_POST, handleSetupSave);
    server.on("/api/status", HTTP_GET, handleStatus);

    server.on("/api/mesh/peers", HTTP_GET, handleMeshPeers);
    server.on("/api/mesh/exec", HTTP_POST, handleMeshExec);
    server.on("/api/mesh/scripts", HTTP_POST, handleMeshScripts);
    server.on("/api/mesh/api", HTTP_GET, handleMeshApi);
    server.on("/api/elk/api", HTTP_GET, handleElkApi);

    server.onNotFound([]() {
        String uri = server.uri();
        if (uri.startsWith("/api/scripts/") && uri.endsWith("/run") && server.method() == HTTP_POST) {
            handleScriptRun();
            return;
        }
        if (uri.startsWith("/api/scripts/") && server.method() == HTTP_GET) {
            handleScriptLoad();
            return;
        }
        if (uri.startsWith("/api/scripts/") && server.method() == HTTP_DELETE) {
            handleScriptDelete();
            return;
        }
        server.send(404, "application/json", "{\"error\":\"Not found\"}");
    });

    routesRegistered = true;
}

// ─── Task entry point (runs on Core 0) ──────────────────────────────────────
void webApiTask(void* param) {
    (void)param;
    taskShouldRun = true;
    taskRunning = true;

    scriptStoreInit();
    registerRoutes();
    server.begin();
    DBG("api", "HTTP server on port %d (Core %d)", HTTP_PORT, xPortGetCoreID());

    while (taskShouldRun) {
        server.handleClient();
        meshTick();
        vTaskDelay(1);
    }

    server.stop();
    taskRunning = false;
    DBG("api", "HTTP server stopped");
    vTaskDelete(nullptr);
}

void webApiStop() {
    taskShouldRun = false;
}

bool webApiIsRunning() {
    return taskRunning;
}

void webApiSetSetupMode(bool enabled) {
    setupModeActive = enabled;
}

bool webApiIsSetupMode() {
    return setupModeActive;
}

bool webApiConsumeRestartRequest() {
    if (!restartRequested) return false;
    restartRequested = false;
    return true;
}
