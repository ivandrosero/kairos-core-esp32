#include "board.h"
#include <WiFi.h>
#include <esp_heap_caps.h>
#include "config_store.h"

// ─── Display instance ───────────────────────────────────────────────────────
Display_t display(GxEPD2_420(Pin::kEpdCs, Pin::kEpdDc,
                              Pin::kEpdReset, Pin::kEpdBusy));

// ─── GPIO setup ─────────────────────────────────────────────────────────────
void initBoard() {
    pinMode(Pin::kEpdEnable,     OUTPUT);
    pinMode(Pin::kEpdReset,      OUTPUT);
    pinMode(Pin::kSdEnable,      OUTPUT);
    pinMode(Pin::kBatteryEnable, OUTPUT);
    pinMode(Pin::kPcfInterrupt,  INPUT);

    pinMode(Pin::kBtnMenu,    INPUT_PULLUP);
    pinMode(Pin::kBtnNext,    INPUT_PULLUP);
    pinMode(Pin::kBtnSelect,  INPUT_PULLUP);
    pinMode(Pin::kBtnWeather, INPUT_PULLUP);
}

// ─── EPD power-on sequence ──────────────────────────────────────────────────
void enableDisplayPower() {
    digitalWrite(Pin::kEpdEnable, LOW);
    digitalWrite(Pin::kEpdReset,  LOW);
    delay(50);
    digitalWrite(Pin::kEpdReset,  HIGH);
    delay(50);
}

// ─── WiFi with timeout ──────────────────────────────────────────────────────
bool connectWiFi() {
    KairosConfig& cfg = configGet();
    String ssid = cfg.wifi_ssid;
    ssid.trim();
    if (ssid.length() == 0) {
        DBG("wifi", "No WiFi SSID configured");
        return false;
    }

    DBG("wifi", "Connecting to %s ...", ssid.c_str());
    WiFi.mode(WIFI_OFF);
    delay(50);
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);  // Required for ESP-NOW and STA concurrent operation
    WiFi.begin(cfg.wifi_ssid.c_str(), cfg.wifi_pass.c_str());

    unsigned long deadline = millis() + WIFI_TIMEOUT_MS;
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() > deadline) {
            DBG("wifi", "Timed out after %lu ms", WIFI_TIMEOUT_MS);
            return false;
        }
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    DBG("wifi", "Connected — IP %s", WiFi.localIP().toString().c_str());
    return true;
}

bool startSetupAccessPoint(const char* ssid) {
    WiFi.mode(WIFI_OFF);
    delay(50);
    WiFi.mode(WIFI_AP);
    WiFi.setSleep(false);

    if (!WiFi.softAP(ssid)) {
        DBG("wifi", "Failed to start setup AP '%s'", ssid);
        return false;
    }

    delay(100);
    DBG("wifi", "Setup AP '%s' ready at %s", ssid, WiFi.softAPIP().toString().c_str());
    return true;
}

bool buttonDown(uint8_t pin) {
    return digitalRead(pin) == LOW;
}

bool buttonsHeld(uint8_t pinA, uint8_t pinB, uint32_t holdMs) {
    if (!buttonDown(pinA) || !buttonDown(pinB)) return false;

    uint32_t startMs = millis();
    while (millis() - startMs < holdMs) {
        if (!buttonDown(pinA) || !buttonDown(pinB)) return false;
        delay(10);
    }

    while (buttonDown(pinA) || buttonDown(pinB)) {
        delay(10);
    }
    return true;
}

// ─── Button debounce ────────────────────────────────────────────────────────
bool buttonPressed(uint8_t pin) {
    if (buttonDown(pin)) {
        delay(50);
        if (buttonDown(pin)) {
            while (buttonDown(pin)) delay(10);
            return true;
        }
    }
    return false;
}

// ─── Heap diagnostic ────────────────────────────────────────────────────────
void logHeap(const char* tag) {
    DBG(tag, "Free heap: %u bytes", (unsigned)heap_caps_get_free_size(MALLOC_CAP_8BIT));
}
