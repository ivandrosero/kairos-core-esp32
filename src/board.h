#pragma once
// ─── Board-level hardware: display object, pin init, power, buttons, WiFi ───

#include <GxEPD2_BW.h>
#include "config.h"

using Display_t = GxEPD2_BW<GxEPD2_420, GxEPD2_420::HEIGHT>;
extern Display_t display;

void initBoard();
void enableDisplayPower();
bool connectWiFi();
bool startSetupAccessPoint(const char* ssid);
bool buttonDown(uint8_t pin);
bool buttonsHeld(uint8_t pinA, uint8_t pinB, uint32_t holdMs);
bool buttonPressed(uint8_t pin);
void logHeap(const char* tag);
