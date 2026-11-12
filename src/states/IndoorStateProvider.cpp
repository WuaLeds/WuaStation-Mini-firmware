#include "states/IndoorStateProvider.h"

#include <math.h>
#include "config/Config.h"
#include "config/Theme.h"
#include "display/Icons.h"
#include "display/TinyFont.h"
#include "display/Widgets.h"

IndoorStateProvider::IndoorStateProvider(Compositor &compositor, const SensorManager &sensors)
    : StaticStateBase(compositor, "indoor", Config::kIndoorScreenMs), _sensors(sensors)
{
}

uint8_t IndoorStateProvider::phaseCount() const
{
    return 1 + (_reading.hasHumidity ? 1 : 0) + (_reading.hasPressure ? 1 : 0);
}

uint8_t IndoorStateProvider::currentPhase() const
{
    const uint8_t phases = phaseCount();
    const uint8_t phase = static_cast<uint8_t>(shownMs() / (Config::kIndoorScreenMs / phases));
    return phase >= phases ? phases - 1 : phase;
}

void IndoorStateProvider::onShow()
{
    _reading = _sensors.reading();
    _version = _sensors.version();
    _phase = 0;
}

void IndoorStateProvider::tick()
{
    if (_sensors.version() != _version && _sensors.hasReading())
    {
        _reading = _sensors.reading();
        _version = _sensors.version();
        invalidate();
    }
    const uint8_t phase = currentPhase();
    if (phase != _phase)
    {
        _phase = phase;
        invalidate();
    }
}

void IndoorStateProvider::render(Adafruit_GFX &canvas)
{
    char text[8];

    // Phase order: temperature, humidity (if any), pressure (if any).
    uint8_t phase = _phase;
    if (phase == 0)
    {
        const int t = static_cast<int>(lroundf(_reading.temperatureC));
        if (t <= -10)
            snprintf(text, sizeof(text), "%d", t);
        else
            snprintf(text, sizeof(text), "%d%c", t, TinyFont::kDegree);
        Widgets::iconAndValue(canvas, Icons::kHouse, text, Theme::kIndoorTemp.to565());
        return;
    }
    if (!_reading.hasHumidity)
        ++phase; // skip the humidity slot
    if (phase == 1)
    {
        snprintf(text, sizeof(text), "%d%%", static_cast<int>(lroundf(_reading.humidity)));
        Widgets::iconAndValue(canvas, Icons::kDrop, text, Theme::kHumidity.to565());
        return;
    }
    // Pressure: "1013" (hPa) is 15 px wide, no room for an icon.
    snprintf(text, sizeof(text), "%d", static_cast<int>(lroundf(_reading.pressureHpa)));
    TinyFont::drawCentered(canvas, text, Theme::kPressure.to565());
}
