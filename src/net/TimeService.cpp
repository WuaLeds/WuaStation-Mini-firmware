#include "net/TimeService.h"

#include <sys/time.h>
#include "config/Config.h"
#include "core/Logger.h"
#include "net/Timezones.h"

namespace
{
    // Anything before 2024-01-01 means "the RTC was never set".
    constexpr time_t kValidEpoch = 1704067200;
}

void TimeService::begin(const char *timezoneSetting, const char *lastLocationZone)
{
    _auto = Timezones::isAuto(timezoneSetting);
    if (_auto)
    {
        // Known once the weather location reports its zone.
        _timezoneKnown = true;
        apply(lastLocationZone && *lastLocationZone ? lastLocationZone : "UTC0");
        LOG_INFO("Timezone auto (weather location), starting with '%s'", _posix);
        return;
    }

    const char *posix = Timezones::toPosix(timezoneSetting);
    _timezoneKnown = posix != nullptr;
    apply(posix ? posix : "UTC0");
    LOG_INFO("Timezone '%s' -> '%s'%s", timezoneSetting, _posix, _timezoneKnown ? "" : " (unknown, UTC)");
}

bool TimeService::applyLocationZone(const char *ianaName, int32_t utcOffsetSeconds)
{
    if (!_auto)
        return false;

    char posix[sizeof(_posix)];
    const char *known = (ianaName && *ianaName) ? Timezones::toPosix(ianaName) : nullptr;
    if (known)
        strlcpy(posix, known, sizeof(posix));
    else
        Timezones::fromUtcOffset(utcOffsetSeconds, posix, sizeof(posix));

    if (strcmp(posix, _posix) == 0)
        return false;
    apply(posix);
    LOG_INFO("Timezone auto: '%s' (UTC%+ld s) -> '%s'", ianaName ? ianaName : "",
             static_cast<long>(utcOffsetSeconds), _posix);
    return true;
}

void TimeService::apply(const char *posix)
{
    strlcpy(_posix, posix, sizeof(_posix));
    setenv("TZ", _posix, 1);
    tzset();
}

void TimeService::startSync()
{
    if (_syncStarted)
        return;
    _syncStarted = true;
    configTzTime(_posix, Config::kNtpServer1, Config::kNtpServer2, Config::kNtpServer3);
    LOG_INFO("SNTP started");
}

bool TimeService::isSynced() const
{
    return time(nullptr) >= kValidEpoch;
}

bool TimeService::localTime(struct tm &out) const
{
    if (!isSynced())
        return false;
    const time_t now = time(nullptr);
    localtime_r(&now, &out);
    return true;
}

bool TimeService::firstHalfSecond() const
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return tv.tv_usec < 500000;
}
