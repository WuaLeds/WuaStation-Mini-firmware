#pragma once

#include <Adafruit_GFX.h>

// =============================================================================
//  TinyFont -- compact 7-pixel-tall font for the 18x7 panel.
//
//  The classic 5x7 GFX font only fits 3 characters across 18 pixels, which is
//  great for scrolling but too wide for static screens. TinyFont packs digits
//  into 3 columns so "12:34", "21.11", "23°" or "45%" fit on one screen.
//
//  Supported: 0-9 : . - + % / space and the degree sign (TinyFont::kDegree).
//  Unsupported characters are skipped.
// =============================================================================
namespace TinyFont
{
    constexpr int16_t kHeight = 7;
    constexpr int16_t kSpacing = 1;
    constexpr char kDegree = '\xB0'; // Latin-1 degree sign, used as a key

    // Width in pixels of one glyph (0 if unsupported).
    int16_t charWidth(char c);

    // Width in pixels of a string, including the spacing between glyphs.
    int16_t textWidth(const char *text);

    // Draw `text` with its top-left corner at (x, y). Returns the x where a
    // following glyph would start (end of text + spacing).
    int16_t draw(Adafruit_GFX &g, int16_t x, int16_t y, const char *text, uint16_t color);

    // Draw `text` centered inside the horizontal area [areaX, areaX + areaW).
    // areaW < 0 means "up to the right edge of the target".
    void drawCentered(Adafruit_GFX &g, const char *text, uint16_t color,
                      int16_t areaX = 0, int16_t areaW = -1, int16_t y = 0);
}
