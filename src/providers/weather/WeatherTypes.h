#pragma once

#include <Arduino.h>
#include "config/Config.h"

// High-level weather condition, decoupled from any specific API: the display
// only needs the general "look" of the weather to pick an icon.
enum class WeatherCondition : uint8_t
{
    Unknown,
    Clear,
    PartlyCloudy,
    Cloudy,
    Fog,
    Rain,
    Snow,
    Thunderstorm
};

// Plain data: shared between tasks via SharedState.
struct WeatherData
{
    // ---- Current conditions ----
    bool valid;
    float temperatureC;
    uint8_t humidity;     // relative humidity, 0..100 %
    uint16_t weatherCode; // raw provider code (WMO), for debugging
    WeatherCondition condition;
    bool isDay;
    char place[32]; // resolved location name
    uint32_t updatedMs;

    // ---- Timezone of the location (drives the clock in "auto" mode) ----
    bool hasTimezone;
    char zoneName[40];  // IANA name, e.g. "America/Argentina/Buenos_Aires"
    int32_t utcOffsetS; // current offset from UTC, DST included

    // ---- Today's forecast ----
    bool hasMinMax;
    float maxC;
    float minC;
    bool hasRainChance;
    uint8_t rainChance; // max probability of precipitation today, %
    bool hasUv;
    float uvMax; // max UV index today

    // ---- Air quality (separate API, may be missing) ----
    bool hasAirQuality;
    uint16_t usAqi;
    uint16_t europeanAqi;
};
