#include "states/ForecastStateProvider.h"

#include <math.h>
#include "config/Config.h"
#include "config/Theme.h"
#include "display/ColorScale.h"
#include "display/Icons.h"
#include "display/Widgets.h"

ForecastStateProvider::ForecastStateProvider(Compositor &compositor, const OpenMeteoWeatherProvider &provider)
    : PagedStateBase(compositor, "forecast", Config::kForecastScreenMs), _provider(provider)
{
}

uint8_t ForecastStateProvider::beginPages()
{
    _provider.read(_data);
    uint8_t n = 0;
    if (_data.hasMinMax)
    {
        _pages[n++] = Page::Max;
        _pages[n++] = Page::Min;
    }
    if (_data.hasRainChance)
        _pages[n++] = Page::Rain;
    if (_data.hasUv)
        _pages[n++] = Page::Uv;
    if (_data.hasAirQuality)
        _pages[n++] = Page::AirQuality;
    return n;
}

uint32_t ForecastStateProvider::pageDurationMs(uint8_t page) const
{
    (void)page;
    return getDurationMs() / pageCount();
}

void ForecastStateProvider::renderPage(Adafruit_GFX &canvas, uint8_t page, uint32_t elapsedMs)
{
    (void)elapsedMs;
    char text[8];

    switch (_pages[page])
    {
    case Page::Max:
        Widgets::formatTemperature(_data.maxC, text, sizeof(text));
        Widgets::iconAndValue(canvas, Icons::kArrowUp, text, ColorScale::temperature(_data.maxC).to565());
        break;

    case Page::Min:
        Widgets::formatTemperature(_data.minC, text, sizeof(text));
        Widgets::iconAndValue(canvas, Icons::kArrowDown, text, ColorScale::temperature(_data.minC).to565());
        break;

    case Page::Rain:
        snprintf(text, sizeof(text), "%u%%", static_cast<unsigned>(_data.rainChance));
        Widgets::iconAndValue(canvas, Icons::kUmbrella, text, Theme::kRainChance.to565());
        break;

    case Page::Uv:
        snprintf(text, sizeof(text), "%d", static_cast<int>(lroundf(_data.uvMax)));
        Widgets::tagAndValue(canvas, "UV", Theme::kUvTag.to565(), text, ColorScale::uvIndex(_data.uvMax).to565());
        break;

    case Page::AirQuality: {
        const bool european = _provider.europeanAqi();
        const uint16_t aqi = european ? _data.europeanAqi : _data.usAqi;
        snprintf(text, sizeof(text), "%u", static_cast<unsigned>(aqi));
        Widgets::tagAndValue(canvas, "AQ", Theme::kAirTag.to565(), text, ColorScale::airQuality(aqi, european).to565());
        break;
    }
    }
}
