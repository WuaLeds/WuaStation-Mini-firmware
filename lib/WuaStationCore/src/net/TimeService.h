#pragma once

#include <Arduino.h>
#include <time.h>

// =============================================================================
//  TimeService -- wall clock from SNTP with timezone / DST handling.
//
//  Uses the ESP-IDF SNTP client running in the background: unlike the ezTime
//  approach of the WuaDisplay-r1 firmware it never blocks the main loop, so
//  scrolling stays smooth while the clock synchronizes.
//
//  Timezone "auto": the clock follows the timezone of the weather location
//  (Open-Meteo reports it with every forecast). Until the first forecast
//  arrives the last zone seen is used (kept in NVS by the caller).
// =============================================================================
class TimeService
{
public:
    // Resolve and apply the timezone setting (works before any network).
    // `lastLocationZone`: POSIX zone remembered from a previous auto run.
    void begin(const char *timezoneSetting, const char *lastLocationZone);

    // Start SNTP. Call when the network comes up; safe to call repeatedly.
    void startSync();

    // Auto mode only: apply the zone of the weather location (IANA name,
    // with the current UTC offset as fallback for names missing from the
    // built-in table). Returns true when the zone changed.
    bool applyLocationZone(const char *ianaName, int32_t utcOffsetSeconds);

    bool isSynced() const;

    // Local broken-down time. Returns false while not synchronized.
    bool localTime(struct tm &out) const;

    // True during the first half of every second (blinking colons).
    bool firstHalfSecond() const;

    bool isAuto() const { return _auto; }
    bool timezoneKnown() const { return _timezoneKnown; }
    const char *posixTimezone() const { return _posix; }

private:
    void apply(const char *posix);

    char _posix[64] = "UTC0";
    bool _auto = false;
    bool _timezoneKnown = false;
    bool _syncStarted = false;
};
