// ─── script_store.cpp — persist scripts (SD → LittleFS fallback) ────────────
// Ported from famiglia-ink script_manager.cpp

#include "script_store.h"
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <LittleFS.h>
#include "config.h"

static const char* SCRIPTS_DIR = "/scripts";
static constexpr size_t MAX_SCRIPT_SIZE = 32768;
static bool fsReady = false;
static fs::FS* scriptFS = nullptr;

static String scriptPath(const String& name, const String& type) {
    return String(SCRIPTS_DIR) + "/" + name + "." + type;
}

static String findType(const String& name) {
    if (!scriptFS) return "";
    if (scriptFS->exists(scriptPath(name, "bly"))) return "bly";
    if (scriptFS->exists(scriptPath(name, "js")))  return "js";
    return "";
}

static bool isValidName(const String& name) {
    if (name.length() == 0 || name.length() > 32) return false;
    for (size_t i = 0; i < name.length(); ++i) {
        char c = name[i];
        if (!isalnum(c) && c != '-' && c != '_') return false;
    }
    return true;
}

void scriptStoreInit() {
    if (fsReady) return;

    // Try SD card first
    digitalWrite(Pin::kSdEnable, LOW);
    delay(50);
    if (SD.begin(Pin::kSdEnable)) {
        scriptFS = &SD;
        fsReady = true;
        if (!SD.exists(SCRIPTS_DIR)) SD.mkdir(SCRIPTS_DIR);
        DBG("store", "Script store ready (SD, %lluMB)",
            SD.totalBytes() / (1024ULL * 1024ULL));
        return;
    }

    // SD failed — deassert CS to free SPI bus for display
    SD.end();
    digitalWrite(Pin::kSdEnable, HIGH);

    // Fallback to LittleFS
    if (LittleFS.begin(true)) {
        scriptFS = &LittleFS;
        fsReady = true;
        if (!LittleFS.exists(SCRIPTS_DIR)) LittleFS.mkdir(SCRIPTS_DIR);
        DBG("store", "Script store ready (LittleFS fallback)");
        return;
    }

    DBG("store", "No filesystem available!");
}

bool scriptSave(const String& name, const String& code, const String& type, const String& cron) {
    if (!fsReady) { DBG("store", "FS not ready"); return false; }
    if (!isValidName(name)) {
        DBG("store", "Invalid name: %s", name.c_str());
        return false;
    }
    if (type != "js" && type != "bly") {
        DBG("store", "Invalid type: %s", type.c_str());
        return false;
    }
    if (code.length() > MAX_SCRIPT_SIZE) {
        DBG("store", "Script too large: %u bytes", (unsigned)code.length());
        return false;
    }

    // Remove other format if it exists
    String otherType = (type == "js") ? "bly" : "js";
    String otherPath = scriptPath(name, otherType);
    if (scriptFS->exists(otherPath)) {
        scriptFS->remove(otherPath);
    }

    File f = scriptFS->open(scriptPath(name, type), FILE_WRITE);
    if (!f) {
        DBG("store", "Failed to open %s for writing", name.c_str());
        return false;
    }
    f.print(code);
    f.close();
    DBG("store", "Saved '%s.%s' (%u bytes)", name.c_str(), type.c_str(),
        (unsigned)code.length());

    // Write meta file if cron given, else delete any old meta
    String metaPath = scriptPath(name, "meta");
    if (cron.length() > 0) {
        File mf = scriptFS->open(metaPath, FILE_WRITE);
        if (mf) { mf.print("{\"cron\":\"" + cron + "\"}"); mf.close(); }
    } else {
        if (scriptFS->exists(metaPath)) scriptFS->remove(metaPath);
    }
    return true;
}

String scriptLoad(const String& name, String* type) {
    if (!fsReady) return String();
    if (!isValidName(name)) return String();
    String foundType = findType(name);
    if (foundType.length() == 0) return String();

    File f = scriptFS->open(scriptPath(name, foundType), FILE_READ);
    if (!f) return String();
    String code = f.readString();
    f.close();
    if (type) *type = foundType;
    return code;
}

bool scriptDelete(const String& name) {
    if (!fsReady) return false;
    if (!isValidName(name)) return false;
    bool removed = false;
    String jsPath = scriptPath(name, "js");
    String blyPath = scriptPath(name, "bly");
    if (scriptFS->exists(jsPath))  { scriptFS->remove(jsPath);  removed = true; }
    if (scriptFS->exists(blyPath)) { scriptFS->remove(blyPath); removed = true; }
    return removed;
}

String scriptListJson() {
    if (!fsReady) return "[]";
    String json = "[";
    File dir = scriptFS->open(SCRIPTS_DIR);
    if (!dir || !dir.isDirectory()) return "[]";

    bool first = true;
    File entry = dir.openNextFile();
    while (entry) {
        String fname = entry.name();
        String type = "js";

        if (fname.endsWith(".bly")) {
            type = "bly";
            fname = fname.substring(0, fname.length() - 4);
        } else if (fname.endsWith(".js")) {
            type = "js";
            fname = fname.substring(0, fname.length() - 3);
        } else {
            entry = dir.openNextFile();
            continue;
        }

        int slash = fname.lastIndexOf('/');
        if (slash >= 0) fname = fname.substring(slash + 1);

        if (!first) json += ",";
        json += "{\"name\":\"";
        json += fname;
        json += "\",\"size\":";
        json += entry.size();
        json += ",\"type\":\"";
        json += type;
        json += "\"}";
        first = false;
        entry = dir.openNextFile();
    }
    json += "]";
    return json;
}

String scriptLoadMeta(const String& name) {
    if (!fsReady) return "";
    File f = scriptFS->open(scriptPath(name, "meta"), FILE_READ);
    if (!f) return "";
    String content = f.readString();
    f.close();
    return content;
}
