#pragma once

#include <Arduino.h>

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
    bool valid;
    float temperatureC;
    uint8_t humidity;     // relative humidity, 0..100 %
    uint16_t weatherCode; // raw provider code (WMO), for debugging
    WeatherCondition condition;
    bool isDay;
    char place[32]; // resolved location name
    uint32_t updatedMs;
};
