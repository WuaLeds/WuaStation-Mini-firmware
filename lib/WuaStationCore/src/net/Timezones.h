#pragma once

#include <stddef.h>
#include <stdint.h>

namespace Timezones
{
    // Resolve a timezone setting into a POSIX TZ string usable by the C
    // library (DST rules included).
    //   * Olson names from a built-in table, written loosely: case and
    //     spaces / underscores do not matter, and the city or country alone
    //     is enough ("America/Argentina/Buenos_Aires", "Buenos Aires",
    //     "buenos_aires", "Argentina", "New York").
    //   * Anything without a '/' that carries a digit is taken as a POSIX
    //     string already (e.g. "CET-1CEST,M3.5.0,M10.5.0/3", "<-03>3").
    // Returns nullptr for an unknown name (and for "auto").
    const char *toPosix(const char *setting);

    // "auto" (or empty): follow the timezone of the weather location.
    bool isAuto(const char *setting);

    // Olson names of the built-in table, i = 0.. (nullptr past the end).
    const char *zoneName(size_t i);

    // Fixed-offset POSIX string for a UTC offset: -10800 -> "<-03>3",
    // 19800 -> "<+0530>-5:30". Used for zones missing from the table.
    void fromUtcOffset(int32_t offsetSeconds, char *out, size_t size);
}
