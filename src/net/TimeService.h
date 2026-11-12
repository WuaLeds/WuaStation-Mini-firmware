#pragma once

#include <Arduino.h>
#include <time.h>

// =============================================================================
//  TimeService -- wall clock from SNTP with timezone / DST handling.
//
//  Uses the ESP-IDF SNTP client running in the background: unlike the ezTime
//  approach of the WuaDisplay-r1 firmware it never blocks the main loop, so
//  scrolling stays smooth while the clock synchronizes.
// =============================================================================
class TimeService
{
public:
    // Resolve and apply the timezone setting (works before any network).
    void begin(const char *timezoneSetting);

    // Start SNTP. Call when the network comes up; safe to call repeatedly.
    void startSync();

    bool isSynced() const;

    // Local broken-down time. Returns false while not synchronized.
    bool localTime(struct tm &out) const;

    // True during the first half of every second (blinking colons).
    bool firstHalfSecond() const;

    bool timezoneKnown() const { return _timezoneKnown; }
    const char *posixTimezone() const { return _posix; }

private:
    char _posix[64] = "UTC0";
    bool _timezoneKnown = false;
    bool _syncStarted = false;
};
