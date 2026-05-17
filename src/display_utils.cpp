#include "display_utils.h"
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>

// ─── Scene lifecycle ────────────────────────────────────────────────────────
void beginScene() {
    display.setRotation(0);
    display.setFullWindow();
    display.fillScreen(GxEPD_WHITE);
}

// ─── Font presets ───────────────────────────────────────────────────────────
void useBodyFont() {
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextSize(1);
    display.setTextColor(GxEPD_BLACK);
    display.setTextWrap(false);
}

void useTitleFont() {
    display.setFont(&FreeMonoBold12pt7b);
    display.setTextSize(1);
    display.setTextColor(GxEPD_BLACK);
    display.setTextWrap(false);
}

// ─── Centred text block inside a rectangle ──────────────────────────────────
void drawCenteredTextLines(const String* lines, size_t count,
                           int rectX, int rectY, int rectW, int rectH,
                           int lineGap) {
    int16_t tx, ty;
    uint16_t sampleW, sampleH;
    display.getTextBounds("Ag", 0, 0, &tx, &ty, &sampleW, &sampleH);

    int lineAdv   = static_cast<int>(sampleH) + lineGap;
    int blockH    = (count > 0) ? static_cast<int>(count) * lineAdv - lineGap : 0;
    int baselineY = rectY + max(lineAdv, (rectH - blockH) / 2 + lineAdv - 2);

    for (size_t i = 0; i < count; ++i) {
        uint16_t lineW = 0, lineH = sampleH;
        if (lines[i].length() > 0) {
            display.getTextBounds(lines[i].c_str(), 0, 0, &tx, &ty, &lineW, &lineH);
        }
        int cx = rectX + max(0, (rectW - static_cast<int>(lineW)) / 2);
        int cy = baselineY + static_cast<int>(i) * lineAdv;
        display.setCursor(cx, cy);
        display.print(lines[i]);
    }
}

// ─── Full-screen centred text ───────────────────────────────────────────────
void displayText(const char* text) {
    const int w = display.width();
    const int h = display.height();
    String content(text ? text : "");

    size_t lineCount = 1;
    for (size_t i = 0; i < content.length(); ++i) {
        if (content[i] == '\n') lineCount++;
    }

    String* lines = new String[lineCount];
    size_t idx = 0;
    int start = 0;
    while (start <= static_cast<int>(content.length()) && idx < lineCount) {
        int end = content.indexOf('\n', start);
        if (end < 0) end = content.length();
        lines[idx++] = content.substring(start, end);
        if (end >= static_cast<int>(content.length())) break;
        start = end + 1;
    }

    beginScene();
    useBodyFont();
    drawCenteredTextLines(lines, lineCount, 0, 0, w, h, 8);
    display.display(false);
    delete[] lines;
}
