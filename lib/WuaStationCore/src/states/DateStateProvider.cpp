#include "states/DateStateProvider.h"

#include "config/Config.h"
#include "config/Theme.h"
#include "display/TinyFont.h"

DateStateProvider::DateStateProvider(Compositor &compositor, const TimeService &time)
    : StaticStateBase(compositor, "date", Config::kDateScreenMs), _time(time)
{
}

void DateStateProvider::tick()
{
    struct tm now;
    if (_time.localTime(now) && now.tm_yday != _shownDay)
        invalidate();
}

void DateStateProvider::render(Adafruit_GFX &canvas)
{
    struct tm now;
    if (!_time.localTime(now))
        return;
    _shownDay = now.tm_yday;

    char text[8];
    snprintf(text, sizeof(text), "%02d.%02d", now.tm_mday, now.tm_mon + 1);
    TinyFont::drawCentered(canvas, text, Theme::kDate.to565());
}
