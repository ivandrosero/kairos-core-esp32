#pragma once
// ─── Persistent configuration on LittleFS ───────────────────────────────────

#include <Arduino.h>

struct KairosConfig {
    String wifi_ssid;
    String wifi_pass;
    bool   ws_enabled;
    String ws_host;
    uint16_t ws_port;
    String ws_path;
    bool   ws_ssl;
    String device_name;
    String ntp_server;
    String tz_info;
    // MQTT
    bool     mqtt_enabled;
    String   mqtt_broker;
    uint16_t mqtt_port;
    String   mqtt_user;
    String   mqtt_pass;
    String   mqtt_topic_prefix;
};

void configInit();
KairosConfig& configGet();
bool configHasWifiCredentials();
bool configSave();
String configToJson(bool maskPasswords = true);
bool configMergeJson(const String& json);
bool configRequestSetupMode();
bool configConsumeSetupModeRequest();
