#pragma once

#include "net/TimeService.h"
#include "states/ScrollTextStateBase.h"

// =============================================================================
//  Calendar event (ported from WuaDisplay-r1): on the target day the message
//  scrolls once per rotation. Override isActiveDate() / buildMessage() for
//  other kinds of events (see MonthlyCelebrationStateProvider).
// =============================================================================
class CalendarStateProvider : public ScrollTextStateBase
{
public:
    struct Date
    {
        int year;  // e.g. 2025
        int month; // 1..12
        int day;   // 1..31
    };

    CalendarStateProvider(Compositor &compositor, const TimeService &time, const Date &target,
                          const char *id = "calendar", Rgb color = Theme::kCelebration);

    bool canBeShown() const override;

protected:
    bool nextPass(String &text, Rgb &color) override;

    virtual bool isActiveDate(const struct tm &now) const;
    virtual String buildMessage(const struct tm &now) const;

    const Date &targetDate() const { return _target; }

private:
    const TimeService &_time;
    Date _target;
    Rgb _color;
};
