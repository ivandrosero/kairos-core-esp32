#pragma once
// ─── Compile-time configuration ─────────────────────────────────────────────

#include <Arduino.h>
#include "secrets.h"

// ─── Debug logging ──────────────────────────────────────────────────────────
#define DEBUG_ENABLED 1

#if DEBUG_ENABLED
  #define DBG(tag, fmt, ...) do { Serial.printf("[%s] " fmt "\n", tag, ##__VA_ARGS__); } while(0)
  #define DBG_PLAIN(fmt, ...) do { Serial.printf(fmt "\n", ##__VA_ARGS__); } while(0)
#else
  #define DBG(tag, fmt, ...) ((void)0)
  #define DBG_PLAIN(fmt, ...) ((void)0)
#endif

// ─── Hardware pins (Paperd.ink Classic) ─────────────────────────────────────
namespace Pin {
    constexpr uint8_t kEpdCs         = 22;
    constexpr uint8_t kEpdDc         = 15;
    constexpr uint8_t kEpdBusy       = 34;
    constexpr uint8_t kEpdEnable     = 12;
    constexpr uint8_t kEpdReset      = 13;
    constexpr uint8_t kSdEnable      = 5;
    constexpr uint8_t kBatteryEnable = 25;
    constexpr uint8_t kPcfInterrupt  = 35;

    constexpr uint8_t kBtnMenu    = 14;   // top
    constexpr uint8_t kBtnNext    = 27;   // second
    constexpr uint8_t kBtnSelect  = 4;    // third
    constexpr uint8_t kBtnWeather = 2;    // bottom
}

// ─── Network ────────────────────────────────────────────────────────────────
constexpr uint16_t      HTTP_PORT        = 80;
constexpr unsigned long WIFI_TIMEOUT_MS  = 15000;

// ─── Scheduler ──────────────────────────────────────────────────────────────
constexpr uint32_t SCHEDULER_TICK_MS = 100;

// ─── Elk JS Engine ──────────────────────────────────────────────────────────
constexpr size_t ELK_MEM_SIZE = 16384;

// ─── Config file path ───────────────────────────────────────────────────────
#define CONFIG_FILE_PATH "/config.json"
