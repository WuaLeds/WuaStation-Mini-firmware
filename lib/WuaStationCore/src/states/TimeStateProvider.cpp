#include "states/TimeStateProvider.h"

#include "config/Config.h"
#include "config/Theme.h"
#include "display/TinyFont.h"

TimeStateProvider::TimeStateProvider(Compositor &compositor, const TimeService &time)
    : StaticStateBase(compositor, "time", Config::kTimeScreenMs), _time(time)
{
}

void TimeStateProvider::tick()
{
    struct tm now;
    if (!_time.localTime(now))
        return;
    const int key = (now.tm_hour * 60 + now.tm_min) * 2 + (_time.firstHalfSecond() ? 1 : 0);
    if (key != _shownKey)
        invalidate();
}

void TimeStateProvider::render(Adafruit_GFX &canvas)
{
    struct tm now;
    if (!_time.localTime(now))
        return;
    const bool colon = _time.firstHalfSecond();
    _shownKey = (now.tm_hour * 60 + now.tm_min) * 2 + (colon ? 1 : 0);

    char hh[3], mm[3];
    snprintf(hh, sizeof(hh), "%02d", now.tm_hour);
    snprintf(mm, sizeof(mm), "%02d", now.tm_min);

    // "HH:MM" = 3+1+3 +1+ 1 +1+ 3+1+3 = 17 px
    const int16_t total = TinyFont::textWidth("00:00");
    int16_t x = (canvas.width() - total) / 2;
    x = TinyFont::draw(canvas, x, 0, hh, Theme::kClock.to565());
    if (colon)
        TinyFont::draw(canvas, x, 0, ":", Theme::kClockColon.to565());
    x += TinyFont::charWidth(':') + TinyFont::kSpacing;
    TinyFont::draw(canvas, x, 0, mm, Theme::kClock.to565());
}
