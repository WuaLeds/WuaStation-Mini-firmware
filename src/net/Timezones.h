#pragma once

namespace Timezones
{
    // Resolve a timezone setting into a POSIX TZ string usable by the C
    // library (DST rules included).
    //   * Known Olson names ("Europe/Madrid") are looked up in a built-in table.
    //   * Anything without a '/' is treated as a POSIX string already
    //     (e.g. "CET-1CEST,M3.5.0,M10.5.0/3", "<-03>3", "UTC0").
    // Returns nullptr for an unknown Olson name.
    const char *toPosix(const char *setting);
}
