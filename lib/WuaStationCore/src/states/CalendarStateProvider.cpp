#include "states/CalendarStateProvider.h"

#include "config/Strings.h"

CalendarStateProvider::CalendarStateProvider(Compositor &compositor, const TimeService &time, const Date &target,
                                             const char *id, Rgb color)
    : ScrollTextStateBase(compositor, id, 0, 1), _time(time), _target(target), _color(color)
{
}

bool CalendarStateProvider::canBeShown() const
{
    struct tm now;
    return _time.localTime(now) && isActiveDate(now);
}

bool CalendarStateProvider::nextPass(String &text, Rgb &color)
{
    struct tm now;
    if (!_time.localTime(now))
        return false;
    text = buildMessage(now);
    color = _color;
    return true;
}

bool CalendarStateProvider::isActiveDate(const struct tm &now) const
{
    return now.tm_mon + 1 == _target.month && now.tm_mday == _target.day;
}

String CalendarStateProvider::buildMessage(const struct tm &now) const
{
    (void)now;
    char buf[48];
    snprintf(buf, sizeof(buf), Strings::kCalendarFmt, _target.year, _target.month, _target.day);
    return String(buf);
}
