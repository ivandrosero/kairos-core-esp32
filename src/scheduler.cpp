// ─── scheduler.cpp — Core 1 task: dequeue and execute scripts ───────────────

#include "scheduler.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <time.h>
#include "config.h"
#include "ipc.h"
#include "elk_engine.h"
#include "display_utils.h"
#include "script_store.h"
#include <ArduinoJson.h>

// ─── Cron field matcher ─────────────────────────────────────────────────────
// Supports: *  N  N-M  */step  N-M/step  N,M,O  (comma-separated combos)
static bool cronFieldMatch(const String& field, int value, int minVal, int maxVal) {
    if (field == "*") return true;

    // Walk comma-separated parts
    int pos = 0;
    while (pos <= (int)field.length()) {
        int comma = field.indexOf(',', pos);
        if (comma < 0) comma = field.length();
        String part = field.substring(pos, comma);
        pos = comma + 1;

        // Split on '/'  →  base / step
        int step = 1;
        int slash = part.indexOf('/');
        if (slash >= 0) {
            step = part.substring(slash + 1).toInt();
            if (step <= 0) step = 1;
            part = part.substring(0, slash);
        }

        // Split on '-'  →  range start..end
        int dash = part.indexOf('-');
        if (dash >= 0) {
            int lo = part.substring(0, dash).toInt();
            int hi = part.substring(dash + 1).toInt();
            if (value >= lo && value <= hi && (value - lo) % step == 0)
                return true;
        } else if (part == "*") {
            // */step
            if ((value - minVal) % step == 0)
                return true;
        } else {
            // Single value (step ignored for bare numbers)
            if (part.toInt() == value)
                return true;
        }
    }
    return false;
}

// Match a full 5-field cron expression against a struct tm
static bool cronMatch(const String& cron, const struct tm* ti) {
    // Tokenise "MIN HR DOM MON DOW"
    int s1 = cron.indexOf(' ');
    if (s1 < 0) return false;
    int s2 = cron.indexOf(' ', s1 + 1);
    if (s2 < 0) return false;
    int s3 = cron.indexOf(' ', s2 + 1);
    if (s3 < 0) return false;
    int s4 = cron.indexOf(' ', s3 + 1);
    if (s4 < 0) return false;

    String mn  = cron.substring(0, s1);
    String hr  = cron.substring(s1 + 1, s2);
    String dom = cron.substring(s2 + 1, s3);
    String mon = cron.substring(s3 + 1, s4);
    String dow = cron.substring(s4 + 1);
    dow.trim();

    return cronFieldMatch(mn,  ti->tm_min,      0, 59)
        && cronFieldMatch(hr,  ti->tm_hour,     0, 23)
        && cronFieldMatch(dom, ti->tm_mday,     1, 31)
        && cronFieldMatch(mon, ti->tm_mon + 1,  1, 12)   // tm_mon is 0-based
        && cronFieldMatch(dow, ti->tm_wday,     0, 6);
}


void schedulerTask(void* param) {
    (void)param;

    elkEngineCreate();
    DBG("sched", "Scheduler running on Core %d", xPortGetCoreID());

    for (;;) {
        ScriptJob* job = nullptr;
        if (xQueueReceive(g_scriptQueue, &job, pdMS_TO_TICKS(SCHEDULER_TICK_MS)) == pdTRUE) {
            if (job) {
                DBG("sched", "Executing '%s' (%u bytes)", job->name, job->codeLen);

                String errorMsg;
                bool ok = elkEngineExec(job->code, job->codeLen, &errorMsg);

                if (!ok) {
                    DBG("sched", "Script '%s' failed: %s", job->name, errorMsg.c_str());
                    String errDisplay = String("Script error:\n") + job->name + "\n" + errorMsg;
                    displayText(errDisplay.c_str());
                }
                free(job);
            }
        }

        
        // ── Cron scheduler: once per minute, check all scripts for cron triggers
        static time_t lastCronMinute = 0;
        time_t now = time(nullptr);
        struct tm* ti = localtime(&now);
        time_t currentMinute = now / 60;

        if (ti->tm_year > 120 && currentMinute != lastCronMinute) {
            // Valid NTP time (year is since 1900, >120 is 2020+)
            lastCronMinute = currentMinute;
            
            // Check scripts directory for meta files
            String listJson = scriptListJson();
            if (listJson != "[]") {
                JsonDocument listDoc;
                if (deserializeJson(listDoc, listJson) == DeserializationError::Ok) {
                    JsonArray arr = listDoc.as<JsonArray>();
                    for (JsonVariant v : arr) {
                        String name = v["name"].as<String>();
                        String metaStr = scriptLoadMeta(name);
                        if (metaStr.length() > 0) {
                            JsonDocument metaDoc;
                            if (deserializeJson(metaDoc, metaStr) == DeserializationError::Ok) {
                                if (metaDoc["cron"].is<const char*>()) {
                                    String cron = metaDoc["cron"].as<String>();
                                    if (cron.length() > 0 && cronMatch(cron, ti)) {
                                        DBG("sched", "Cron trigger for %s at %02d:%02d",
                                            name.c_str(), ti->tm_hour, ti->tm_min);
                                        String code = scriptLoad(name);
                                        if (code.length() > 0) {
                                            enqueueCode(name.c_str(), code.c_str(), code.length());
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

    }
}


bool enqueueCode(const char* name, const char* code, size_t len) {
    if (len == 0 || len >= SCRIPT_CODE_MAX) {
        DBG("api", "Code too large or empty (%u bytes)", (unsigned)len);
        return false;
    }

    ScriptJob* job = (ScriptJob*)malloc(sizeof(ScriptJob));
    if (!job) {
        DBG("api", "Failed to allocate ScriptJob");
        return false;
    }
    strncpy(job->name, name, SCRIPT_NAME_MAX - 1);
    job->name[SCRIPT_NAME_MAX - 1] = '\0';
    memcpy(job->code, code, len);
    job->code[len] = '\0';
    job->codeLen = len;

    if (xQueueSend(g_scriptQueue, &job, pdMS_TO_TICKS(500)) != pdTRUE) {
        DBG("api", "Script queue full");
        free(job);
        return false;
    }
    return true;
}
