#pragma once

#include <Adafruit_GFX.h>
#include "display/Icons.h"

// Reusable layouts for the 18x7 static screens.
namespace Widgets
{
    // A 7x7 icon on the left and a TinyFont value centered in the space to
    // its right. If the value is too wide, the icon is dropped and the value
    // is centered on the whole panel.
    void iconAndValue(Adafruit_GFX &g, const Icons::Icon &icon, const char *value, uint16_t color);

    // Text in one of the two fonts used on static screens.
    enum class Font : uint8_t
    {
        Classic, // Adafruit GFX 5x7: most legible, 3 characters across
        Tiny     // TinyFont 3x7: compact, ~4-5 characters across
    };

    // Visual width in pixels (no trailing spacing column).
    int16_t textWidth(Font font, const char *text);

    // Draw with the top-left corner at (x, 0).
    void drawText(Adafruit_GFX &g, Font font, int16_t x, const char *text, uint16_t color);
}
