#pragma once

#include <Adafruit_GFX.h>

// =============================================================================
//  7x7 color icons for the 18x7 panel.
//
//  Pixels are written as strings, one char per pixel, using a small palette:
//    '.' off      'Y' yellow   'O' orange   'W' white    'G' gray
//    'B' blue     'C' cyan     'S' snow     'R' red      'M' moon
//  so the artwork stays readable and easy to tweak in the source.
// =============================================================================
namespace Icons
{
    struct Icon
    {
        uint8_t width;
        uint8_t height;
        const char *pixels; // width * height palette chars, row by row
    };

    extern const Icon kSun;
    extern const Icon kMoon;
    extern const Icon kPartlyCloudy;
    extern const Icon kPartlyCloudyNight;
    extern const Icon kCloud;
    extern const Icon kFog;
    extern const Icon kRain;
    extern const Icon kSnow;
    extern const Icon kThunder;
    extern const Icon kUnknown;
    extern const Icon kHouse;
    extern const Icon kDrop;
    extern const Icon kWifi;

    // Draw with the icon's own palette colors.
    void draw(Adafruit_GFX &g, const Icon &icon, int16_t x, int16_t y);

    // Draw every lit pixel with a single color.
    void drawTinted(Adafruit_GFX &g, const Icon &icon, int16_t x, int16_t y, uint16_t color);
}
