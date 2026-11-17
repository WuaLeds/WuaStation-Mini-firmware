#pragma once

#include "display/Icons.h"
#include "providers/weather/OpenMeteoWeatherProvider.h"
#include "states/StaticStateBase.h"

// Outdoor weather (internet): condition icon + temperature, then the
// humidity during the second half of the screen time.
class WeatherStateProvider : public StaticStateBase
{
public:
    WeatherStateProvider(Compositor &compositor, const OpenMeteoWeatherProvider &provider);

    bool canBeShown() const override { return _provider.hasData(); }

    static const Icons::Icon &iconFor(WeatherCondition condition, bool isDay);

protected:
    void onShow() override;
    void tick() override;
    void render(Adafruit_GFX &canvas) override;

private:
    uint8_t currentPhase() const;

    const OpenMeteoWeatherProvider &_provider;
    WeatherData _data{};
    uint32_t _version = 0;
    uint8_t _phase = 0;
};
