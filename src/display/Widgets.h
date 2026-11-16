#pragma once

#include <Adafruit_GFX.h>
#include "display/Icons.h"

// Reusable layouts for the 18x7 static screens.
namespace Widgets
{
    // ---- Fonts ---------------------------------------------------------------
    enum class Font : uint8_t
    {
        Classic, // Adafruit GFX 5x7: most legible, 3 characters across
        Tiny     // TinyFont 3x7: compact, ~4-5 characters across
    };

    // Visual width in pixels (no trailing spacing column).
    int16_t textWidth(Font font, const char *text);

    // Draw with the top-left corner at (x, 0).
    void drawText(Adafruit_GFX &g, Font font, int16_t x, const char *text, uint16_t color);

    // Classic if it fits the panel, else Tiny if it fits, else Classic (to pan).
    Font bestFont(const char *text);

    // ---- Icon / tag + value --------------------------------------------------
    // A 7x7 icon on the left and a TinyFont value centered in the space to
    // its right. If the value is too wide, the icon is dropped and the value
    // is centered on the whole panel.
    void iconAndValue(Adafruit_GFX &g, const Icons::Icon &icon, const char *value, uint16_t color);

    // Same layout with a short TinyFont tag instead of an icon ("UV 7").
    void tagAndValue(Adafruit_GFX &g, const char *tag, uint16_t tagColor, const char *value,
                     uint16_t valueColor);

    // "23°", "-5°", "-12" (no room for the degree sign below -9).
    void formatTemperature(float celsius, char *out, size_t size);

    // ---- Labels: one text that fits, or pans slowly ---------------------------
    struct Label
    {
        char text[24];
        Font font;
        uint16_t color;
        int16_t width;
    };

    void makeLabel(Label &label, const char *text, uint16_t color, Font font);

    // Extra time a label needs on screen to pan all the way (0 if it fits).
    uint32_t panDurationMs(const Label &label);

    // Horizontal offset after `elapsedMs` on screen: hold, pan, hold.
    int16_t panOffset(const Label &label, uint32_t elapsedMs);

    // Centered when it fits, panned otherwise.
    void drawLabel(Adafruit_GFX &g, const Label &label, uint32_t elapsedMs);
}
