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

void TimeService::begin(const char *timezoneSetting)
{
    const char *posix = Timezones::toPosix(timezoneSetting);
    _timezoneKnown = posix != nullptr;
    strlcpy(_posix, posix ? posix : "UTC0", sizeof(_posix));

    setenv("TZ", _posix, 1);
    tzset();
    LOG_INFO("Timezone '%s' -> '%s'%s", timezoneSetting, _posix, _timezoneKnown ? "" : " (unknown, UTC)");
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
