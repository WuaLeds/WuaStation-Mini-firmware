#pragma once

#include <Arduino.h>
#include "config/Theme.h"

// Colors that encode a value (temperature, UV, air quality...).
namespace ColorScale
{
    // Cold blue -> green -> amber -> hot red.
    Rgb temperature(float celsius);

    // WHO UV index bands: green, yellow, orange, red, violet.
    Rgb uvIndex(float uv);

    // US AQI (0-500) or European AQI (0-100+) category colors.
    Rgb airQuality(uint16_t aqi, bool european);
}
