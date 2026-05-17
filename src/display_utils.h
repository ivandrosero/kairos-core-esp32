#pragma once
// ─── Text rendering and scene helpers for the e-paper display ───────────────

#include <Arduino.h>
#include "board.h"

void beginScene();
void useBodyFont();
void useTitleFont();

void drawCenteredTextLines(const String* lines, size_t count,
                           int rectX, int rectY, int rectW, int rectH,
                           int lineGap = 6);

void displayText(const char* text);
