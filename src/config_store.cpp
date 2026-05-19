// ─── config_store.cpp — persistent config on LittleFS ───────────────────────

#include "config_store.h"
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "config.h"

static KairosConfig cfg;
static bool initialized = false;
static constexpr const char* SETUP_MODE_FLAG_PATH = "/setup_mode.flag";

void configInit() {
    if (initialized) return;

    // Defaults for first boot; WiFi credentials must come from persisted config.
    cfg.wifi_ssid   = "";
    cfg.wifi_pass   = "";
    cfg.ws_enabled  = false;
    cfg.ws_host     = "";
    cfg.ws_port     = 443;
    cfg.ws_path     = "/kairos/{device_id}";
    cfg.ws_ssl      = true;
    cfg.device_name = "kairos-001";
    cfg.ntp_server  = "ch.pool.ntp.org";
    cfg.tz_info     = "CET-1CEST,M3.5.0,M10.5.0/3";


    // Try loading from file
    if (LittleFS.exists(CONFIG_FILE_PATH)) {
        File f = LittleFS.open(CONFIG_FILE_PATH, "r");
        if (f) {
            JsonDocument doc;
            if (deserializeJson(doc, f) == DeserializationError::Ok) {
                if (doc["wifi_ssid"].is<const char*>())   cfg.wifi_ssid   = doc["wifi_ssid"].as<String>();
                if (doc["wifi_pass"].is<const char*>())   cfg.wifi_pass   = doc["wifi_pass"].as<String>();
                if (doc["ws_enabled"].is<bool>())         cfg.ws_enabled  = doc["ws_enabled"];
                if (doc["ws_host"].is<const char*>())     cfg.ws_host     = doc["ws_host"].as<String>();
                if (doc["ws_port"].is<int>())             cfg.ws_port     = doc["ws_port"];
                if (doc["ws_path"].is<const char*>())     cfg.ws_path     = doc["ws_path"].as<String>();
                if (doc["ws_ssl"].is<bool>())             cfg.ws_ssl      = doc["ws_ssl"];
                if (doc["device_name"].is<const char*>()) cfg.device_name = doc["device_name"].as<String>();
                if (doc["ntp_server"].is<const char*>()) cfg.ntp_server = doc["ntp_server"].as<String>();
                if (doc["tz_info"].is<const char*>()) cfg.tz_info = doc["tz_info"].as<String>();

                DBG("cfg", "Loaded config from %s", CONFIG_FILE_PATH);
            }
            f.close();
        }
    } else {
        DBG("cfg", "No config file, using defaults");
    }

    initialized = true;
}

KairosConfig& configGet() {
    return cfg;
}

bool configHasWifiCredentials() {
    String ssid = cfg.wifi_ssid;
    ssid.trim();
    return ssid.length() > 0;
}

bool configSave() {
    JsonDocument doc;
    doc["wifi_ssid"]   = cfg.wifi_ssid;
    doc["wifi_pass"]   = cfg.wifi_pass;
    doc["ws_enabled"]  = cfg.ws_enabled;
    doc["ws_host"]     = cfg.ws_host;
    doc["ws_port"]     = cfg.ws_port;
    doc["ws_path"]     = cfg.ws_path;
    doc["ws_ssl"]      = cfg.ws_ssl;
    doc["device_name"] = cfg.device_name;
    doc["ntp_server"] = cfg.ntp_server;
    doc["tz_info"] = cfg.tz_info;


    File f = LittleFS.open(CONFIG_FILE_PATH, "w");
    if (!f) {
        DBG("cfg", "Failed to open config for writing");
        return false;
    }
    serializeJson(doc, f);
    f.close();
    DBG("cfg", "Config saved");
    return true;
}

String configToJson(bool maskPasswords) {
    JsonDocument doc;
    doc["wifi_ssid"]   = cfg.wifi_ssid;
    doc["wifi_pass"]   = maskPasswords ? "********" : cfg.wifi_pass;
    doc["ws_enabled"]  = cfg.ws_enabled;
    doc["ws_host"]     = cfg.ws_host;
    doc["ws_port"]     = cfg.ws_port;
    doc["ws_path"]     = cfg.ws_path;
    doc["ws_ssl"]      = cfg.ws_ssl;
    doc["device_name"] = cfg.device_name;
    doc["ntp_server"] = cfg.ntp_server;
    doc["tz_info"] = cfg.tz_info;


    String out;
    serializeJson(doc, out);
    return out;
}

bool configMergeJson(const String& json) {
    JsonDocument doc;
    if (deserializeJson(doc, json) != DeserializationError::Ok) {
        return false;
    }

    if (doc["wifi_ssid"].is<const char*>())   cfg.wifi_ssid   = doc["wifi_ssid"].as<String>();
    if (doc["wifi_pass"].is<const char*>())   cfg.wifi_pass   = doc["wifi_pass"].as<String>();
    if (doc["ws_enabled"].is<bool>())         cfg.ws_enabled  = doc["ws_enabled"];
    if (doc["ws_host"].is<const char*>())     cfg.ws_host     = doc["ws_host"].as<String>();
    if (doc["ws_port"].is<int>())             cfg.ws_port     = doc["ws_port"];
    if (doc["ws_path"].is<const char*>())     cfg.ws_path     = doc["ws_path"].as<String>();
    if (doc["ws_ssl"].is<bool>())             cfg.ws_ssl      = doc["ws_ssl"];
    if (doc["device_name"].is<const char*>()) cfg.device_name = doc["device_name"].as<String>();
    if (doc["ntp_server"].is<const char*>())  cfg.ntp_server  = doc["ntp_server"].as<String>();
    if (doc["tz_info"].is<const char*>())      cfg.tz_info     = doc["tz_info"].as<String>();


    return configSave();
}

bool configRequestSetupMode() {
    File f = LittleFS.open(SETUP_MODE_FLAG_PATH, "w");
    if (!f) {
        DBG("cfg", "Failed to write setup mode flag");
        return false;
    }
    f.print('1');
    f.close();
    return true;
}

bool configConsumeSetupModeRequest() {
    if (!LittleFS.exists(SETUP_MODE_FLAG_PATH)) return false;
    LittleFS.remove(SETUP_MODE_FLAG_PATH);
    DBG("cfg", "Consumed setup mode flag");
    return true;
}
