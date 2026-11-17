#pragma once

#include <Arduino.h>
#include <atomic>
#include "core/SharedState.h"
#include "net/NetWorker.h"
#include "providers/weather/WeatherTypes.h"

// =============================================================================
//  Weather from Open-Meteo (free, no API key).
//
//    forecast API    : current conditions, today's max/min, chance of rain,
//                      UV index and the timezone of the location (clock in
//                      "auto" mode)
//    air quality API : US / European AQI (optional: weather works without it)
//
//  Location: explicit latitude/longitude, or a city name resolved once with
//  the Open-Meteo geocoding API ("Madrid", or "Madrid,ES" to pick a country).
//  Runs as an INetJob in the network task.
// =============================================================================
class OpenMeteoWeatherProvider : public INetJob
{
public:
    // Call before NetWorker::begin(). Coordinates win when both are given.
    void configure(const char *city, bool hasCoordinates, float latitude, float longitude, bool europeanAqi);

    const char *name() const override { return "open-meteo"; }
    bool enabled() const override { return _hasCoordinates || _city[0] != '\0'; }
    uint32_t run() override;

    uint32_t read(WeatherData &out) const { return _shared.read(out); }
    uint32_t version() const { return _shared.version(); }

    // Cheap flags for canBeShown() (no snapshot copy).
    bool hasData() const { return _hasData.load(); }
    bool hasForecast() const { return _hasForecast.load(); }
    // Air quality scale chosen in the settings: European AQI instead of US.
    bool europeanAqi() const { return _europeanAqi; }

private:
    bool geocode();
    bool fetchForecast();
    void fetchAirQuality();
    static WeatherCondition conditionFromWmo(uint16_t code);

    char _city[48] = {};
    char _country[3] = {};
    char _place[32] = {};
    float _latitude = 0.0f;
    float _longitude = 0.0f;
    bool _hasCoordinates = false;
    bool _europeanAqi = false;

    WeatherData _work{};
    SharedState<WeatherData> _shared;
    std::atomic<bool> _hasData{false};
    std::atomic<bool> _hasForecast{false};
};
