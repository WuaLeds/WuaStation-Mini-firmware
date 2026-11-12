#include "states/WeatherStateProvider.h"

#include <math.h>
#include "config/Config.h"
#include "config/Theme.h"
#include "display/TinyFont.h"
#include "display/Widgets.h"

WeatherStateProvider::WeatherStateProvider(Compositor &compositor, const OpenMeteoWeatherProvider &provider)
    : StaticStateBase(compositor, "weather", Config::kWeatherScreenMs), _provider(provider)
{
}

const Icons::Icon &WeatherStateProvider::iconFor(WeatherCondition condition, bool isDay)
{
    switch (condition)
    {
    case WeatherCondition::Clear:
        return isDay ? Icons::kSun : Icons::kMoon;
    case WeatherCondition::PartlyCloudy:
        return isDay ? Icons::kPartlyCloudy : Icons::kPartlyCloudyNight;
    case WeatherCondition::Cloudy:
        return Icons::kCloud;
    case WeatherCondition::Fog:
        return Icons::kFog;
    case WeatherCondition::Rain:
        return Icons::kRain;
    case WeatherCondition::Snow:
        return Icons::kSnow;
    case WeatherCondition::Thunderstorm:
        return Icons::kThunder;
    case WeatherCondition::Unknown:
    default:
        return Icons::kUnknown;
    }
}

uint8_t WeatherStateProvider::currentPhase() const
{
    return shownMs() < Config::kWeatherScreenMs / 2 ? 0 : 1;
}

void WeatherStateProvider::onShow()
{
    _version = _provider.read(_data);
    _phase = 0;
}

void WeatherStateProvider::tick()
{
    if (_provider.version() != _version)
    {
        _version = _provider.read(_data);
        invalidate();
    }
    const uint8_t phase = currentPhase();
    if (phase != _phase)
    {
        _phase = phase;
        invalidate();
    }
}

void WeatherStateProvider::render(Adafruit_GFX &canvas)
{
    if (!_data.valid)
        return;

    char text[8];
    if (_phase == 0)
    {
        const int t = static_cast<int>(lroundf(_data.temperatureC));
        // "-12" has no room for the degree sign next to the icon.
        if (t <= -10)
            snprintf(text, sizeof(text), "%d", t);
        else
            snprintf(text, sizeof(text), "%d%c", t, TinyFont::kDegree);
        Widgets::iconAndValue(canvas, iconFor(_data.condition, _data.isDay), text,
                              Theme::kWeatherTemp.to565());
    }
    else
    {
        snprintf(text, sizeof(text), "%u%%", static_cast<unsigned>(_data.humidity));
        Widgets::iconAndValue(canvas, Icons::kDrop, text, Theme::kHumidity.to565());
    }
}
