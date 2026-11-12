#pragma once

#include "states/CalendarStateProvider.h"

// Monthly anniversary (ported from WuaDisplay-r1): every month on the target
// day, shows how many months have passed since the target date.
class MonthlyCelebrationStateProvider : public CalendarStateProvider
{
public:
    MonthlyCelebrationStateProvider(Compositor &compositor, const TimeService &time, const Date &target)
        : CalendarStateProvider(compositor, time, target, "celebration", Theme::kCelebration) {}

protected:
    bool isActiveDate(const struct tm &now) const override;
    String buildMessage(const struct tm &now) const override;

private:
    static int daysInMonth(int year, int month);
};
