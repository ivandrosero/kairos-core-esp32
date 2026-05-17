#pragma once
// ─── Script store: save/load/list/delete scripts on SD or LittleFS ──────────

#include <Arduino.h>

void scriptStoreInit();

bool scriptSave(const String& name, const String& code, const String& type = "js", const String& cron = "");
String scriptLoad(const String& name, String* type = nullptr);
String scriptLoadMeta(const String& name);
bool scriptDelete(const String& name);
String scriptListJson();
