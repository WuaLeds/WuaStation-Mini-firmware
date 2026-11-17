#include "states/MonthlyCelebrationStateProvider.h"

#include "config/Strings.h"

bool MonthlyCelebrationStateProvider::isActiveDate(const struct tm &now) const
{
    return now.tm_mday == targetDate().day;
}

String MonthlyCelebrationStateProvider::buildMessage(const struct tm &now) const
{
    const Date &d = targetDate();
    const int y2 = now.tm_year + 1900;
    const int m2 = now.tm_mon + 1;

    int years = y2 - d.year;
    int months = m2 - d.month;
    int days = now.tm_mday - d.day;
    if (days < 0)
    {
        months -= 1;
        days += daysInMonth(y2, m2 == 1 ? 12 : m2 - 1);
    }
    if (months < 0)
    {
        years -= 1;
        months += 12;
    }

    char buf[128];
    snprintf(buf, sizeof(buf), Strings::kCelebrationFmt, years * 12 + months, years, months);
    return String(buf);
}

int MonthlyCelebrationStateProvider::daysInMonth(int year, int month)
{
    static const uint8_t kDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month < 1 || month > 12)
        return 30;
    const bool leap = (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
    return (month == 2 && leap) ? 29 : kDays[month - 1];
}
