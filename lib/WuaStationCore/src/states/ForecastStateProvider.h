#pragma once

#include "providers/weather/OpenMeteoWeatherProvider.h"
#include "states/PagedStateBase.h"

// =============================================================================
//  Today's forecast as fixed pages (missing values are skipped):
//
//    up arrow + max temperature    down arrow + min temperature
//    umbrella + chance of rain     "UV" + UV index      "AQ" + air quality
//
//  Values are colored by their own scale (temperature, UV band, AQI band).
//  The screen time is shared equally by the pages.
// =============================================================================
class ForecastStateProvider : public PagedStateBase
{
public:
    ForecastStateProvider(Compositor &compositor, const OpenMeteoWeatherProvider &provider);

    bool canBeShown() const override { return _provider.hasForecast(); }

protected:
    uint8_t beginPages() override;
    uint32_t pageDurationMs(uint8_t page) const override;
    void renderPage(Adafruit_GFX &canvas, uint8_t page, uint32_t elapsedMs) override;

private:
    enum class Page : uint8_t
    {
        Max,
        Min,
        Rain,
        Uv,
        AirQuality
    };
    static constexpr uint8_t kMaxPages = 5;

    const OpenMeteoWeatherProvider &_provider;
    WeatherData _data{};
    Page _pages[kMaxPages] = {};
};
