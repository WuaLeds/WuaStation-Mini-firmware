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
}
