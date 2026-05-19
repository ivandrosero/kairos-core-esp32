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

};

void configInit();
KairosConfig& configGet();
bool configHasWifiCredentials();
bool configSave();
String configToJson(bool maskPasswords = true);
bool configMergeJson(const String& json);
bool configRequestSetupMode();
bool configConsumeSetupModeRequest();
