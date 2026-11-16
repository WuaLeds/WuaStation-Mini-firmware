#include "net/Timezones.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

namespace Timezones
{
    namespace
    {
        struct Zone
        {
            const char *name;
            const char *posix;
        };

        // Every zone offered on the settings page (POSIX rules from the IANA
        // tzdata). Any POSIX string also works directly, and "auto" covers
        // every zone through the weather location.
        const Zone kZones[] = {
            {"UTC", "UTC0"},
            {"Etc/UTC", "UTC0"},
            // Americas
            {"America/Argentina/Buenos_Aires", "<-03>3"},
            {"America/Argentina/Cordoba", "<-03>3"},
            {"America/Argentina/Mendoza", "<-03>3"},
            {"America/Buenos_Aires", "<-03>3"},
            {"America/Montevideo", "<-03>3"},
            {"America/Sao_Paulo", "<-03>3"},
            {"America/Asuncion", "<-03>3"},
            {"America/Santiago", "<-04>4<-03>,M9.1.6/24,M4.1.6/24"},
            {"America/La_Paz", "<-04>4"},
            {"America/Caracas", "<-04>4"},
            {"America/Bogota", "<-05>5"},
            {"America/Lima", "<-05>5"},
            {"America/Guayaquil", "<-05>5"},
            {"America/Panama", "EST5"},
            {"America/Costa_Rica", "CST6"},
            {"America/Mexico_City", "CST6"},
            {"America/New_York", "EST5EDT,M3.2.0,M11.1.0"},
            {"America/Toronto", "EST5EDT,M3.2.0,M11.1.0"},
            {"America/Chicago", "CST6CDT,M3.2.0,M11.1.0"},
            {"America/Denver", "MST7MDT,M3.2.0,M11.1.0"},
            {"America/Los_Angeles", "PST8PDT,M3.2.0,M11.1.0"},
            {"America/Vancouver", "PST8PDT,M3.2.0,M11.1.0"},
            {"Pacific/Honolulu", "HST10"},
            // Europe
            {"Europe/Madrid", "CET-1CEST,M3.5.0,M10.5.0/3"},
            {"Atlantic/Canary", "WET0WEST,M3.5.0/1,M10.5.0"},
            {"Europe/Lisbon", "WET0WEST,M3.5.0/1,M10.5.0"},
            {"Europe/London", "GMT0BST,M3.5.0/1,M10.5.0"},
            {"Europe/Dublin", "GMT0IST,M3.5.0/1,M10.5.0"},
            {"Europe/Paris", "CET-1CEST,M3.5.0,M10.5.0/3"},
            {"Europe/Brussels", "CET-1CEST,M3.5.0,M10.5.0/3"},
            {"Europe/Amsterdam", "CET-1CEST,M3.5.0,M10.5.0/3"},
            {"Europe/Berlin", "CET-1CEST,M3.5.0,M10.5.0/3"},
            {"Europe/Zurich", "CET-1CEST,M3.5.0,M10.5.0/3"},
            {"Europe/Rome", "CET-1CEST,M3.5.0,M10.5.0/3"},
            {"Europe/Vienna", "CET-1CEST,M3.5.0,M10.5.0/3"},
            {"Europe/Stockholm", "CET-1CEST,M3.5.0,M10.5.0/3"},
            {"Europe/Warsaw", "CET-1CEST,M3.5.0,M10.5.0/3"},
            {"Europe/Athens", "EET-2EEST,M3.5.0/3,M10.5.0/4"},
            {"Europe/Kyiv", "EET-2EEST,M3.5.0/3,M10.5.0/4"},
            {"Europe/Kiev", "EET-2EEST,M3.5.0/3,M10.5.0/4"},
            {"Europe/Istanbul", "<+03>-3"},
            {"Europe/Moscow", "MSK-3"},
            // Africa
            {"Africa/Casablanca", "<+01>-1"},
            {"Africa/Lagos", "WAT-1"},
            {"Africa/Cairo", "EET-2EEST,M4.5.5/0,M10.5.4/24"},
            {"Africa/Johannesburg", "SAST-2"},
            {"Africa/Nairobi", "EAT-3"},
            // Asia
            {"Asia/Jerusalem", "IST-2IDT,M3.4.4/26,M10.5.0"},
            {"Asia/Riyadh", "<+03>-3"},
            {"Asia/Dubai", "<+04>-4"},
            {"Asia/Kolkata", "IST-5:30"},
            {"Asia/Bangkok", "<+07>-7"},
            {"Asia/Jakarta", "WIB-7"},
            {"Asia/Singapore", "<+08>-8"},
            {"Asia/Hong_Kong", "HKT-8"},
            {"Asia/Shanghai", "CST-8"},
            {"Asia/Manila", "PST-8"},
            {"Asia/Seoul", "KST-9"},
            {"Asia/Tokyo", "JST-9"},
            // Oceania
            {"Australia/Perth", "AWST-8"},
            {"Australia/Brisbane", "AEST-10"},
            {"Australia/Sydney", "AEST-10AEDT,M10.1.0,M4.1.0/3"},
            {"Australia/Melbourne", "AEST-10AEDT,M10.1.0,M4.1.0/3"},
            {"Pacific/Auckland", "NZST-12NZDT,M9.5.0,M4.1.0/3"},
        };

        // Names that do not appear inside the Olson name: other cities of a
        // zone, Spanish names and countries. City and country parts of the
        // names above already match on their own (accents are ignored).
        struct Alias
        {
            const char *alias;
            const char *zone;
        };

        const Alias kAliases[] = {
            // Americas
            {"Rosario", "America/Argentina/Buenos_Aires"},
            {"Rio de Janeiro", "America/Sao_Paulo"},
            {"Brasil", "America/Sao_Paulo"},
            {"Brazil", "America/Sao_Paulo"},
            {"Uruguay", "America/Montevideo"},
            {"Paraguay", "America/Asuncion"},
            {"Chile", "America/Santiago"},
            {"Bolivia", "America/La_Paz"},
            {"Venezuela", "America/Caracas"},
            {"Colombia", "America/Bogota"},
            {"Peru", "America/Lima"},
            {"Quito", "America/Guayaquil"},
            {"Ecuador", "America/Guayaquil"},
            {"San Jose", "America/Costa_Rica"},
            {"Mexico", "America/Mexico_City"},
            {"Ciudad de Mexico", "America/Mexico_City"},
            {"CDMX", "America/Mexico_City"},
            {"Miami", "America/New_York"},
            {"Nueva York", "America/New_York"},
            {"Hawaii", "Pacific/Honolulu"},
            // Europe
            {"Spain", "Europe/Madrid"},
            {"Espana", "Europe/Madrid"},
            {"Barcelona", "Europe/Madrid"},
            {"Canarias", "Atlantic/Canary"},
            {"Las Palmas", "Atlantic/Canary"},
            {"Portugal", "Europe/Lisbon"},
            {"Lisboa", "Europe/Lisbon"},
            {"UK", "Europe/London"},
            {"Londres", "Europe/London"},
            {"Bruselas", "Europe/Brussels"},
            {"Roma", "Europe/Rome"},
            {"Viena", "Europe/Vienna"},
            {"Estocolmo", "Europe/Stockholm"},
            {"Varsovia", "Europe/Warsaw"},
            {"Atenas", "Europe/Athens"},
            {"Estambul", "Europe/Istanbul"},
            {"Moscu", "Europe/Moscow"},
            // Africa / Asia / Oceania
            {"El Cairo", "Africa/Cairo"},
            {"Johannesburgo", "Africa/Johannesburg"},
            {"Tel Aviv", "Asia/Jerusalem"},
            {"Israel", "Asia/Jerusalem"},
            {"Riad", "Asia/Riyadh"},
            {"Mumbai", "Asia/Kolkata"},
            {"Bombay", "Asia/Kolkata"},
            {"New Delhi", "Asia/Kolkata"},
            {"Nueva Delhi", "Asia/Kolkata"},
            {"India", "Asia/Kolkata"},
            {"Yakarta", "Asia/Jakarta"},
            {"Singapur", "Asia/Singapore"},
            {"Beijing", "Asia/Shanghai"},
            {"Pekin", "Asia/Shanghai"},
            {"China", "Asia/Shanghai"},
            {"Seul", "Asia/Seoul"},
            {"Tokio", "Asia/Tokyo"},
            {"Japan", "Asia/Tokyo"},
            {"Japon", "Asia/Tokyo"},
            {"Sidney", "Australia/Sydney"},
        };

        // Next character of a name, folded for a loose comparison: lower
        // case, ' ' and '-' read as '_', and the Latin-1 accented letters of
        // UTF-8 input reduced to their base letter ("São" -> "sao").
        char nextFolded(const char *&p)
        {
            const unsigned char c = static_cast<unsigned char>(*p++);
            if (c == ' ' || c == '-')
                return '_';
            if (c == 0xC3 && *p)
            {
                const unsigned char lower = static_cast<unsigned char>(*p++) | 0x20; // A..Z -> a..z
                if (lower >= 0xA0 && lower <= 0xA5) return 'a';
                if (lower == 0xA7) return 'c';
                if (lower >= 0xA8 && lower <= 0xAB) return 'e';
                if (lower >= 0xAC && lower <= 0xAF) return 'i';
                if (lower == 0xB1) return 'n';
                if (lower >= 0xB2 && lower <= 0xB6) return 'o';
                if (lower >= 0xB9 && lower <= 0xBC) return 'u';
                if (lower == 0xBD || lower == 0xBF) return 'y';
                return '?';
            }
            return static_cast<char>(tolower(c));
        }

        // Does the span [name, nameEnd) equal the whole of `input`?
        bool sameName(const char *name, const char *nameEnd, const char *input)
        {
            while (name < nameEnd && *input)
            {
                if (nextFolded(name) != nextFolded(input))
                    return false;
            }
            return name == nameEnd && *input == '\0';
        }

        // "America/Argentina/Buenos_Aires" matches its full name, any tail
        // after a '/' ("Argentina/Buenos_Aires", "Buenos_Aires") and any
        // inner part ("Argentina").
        bool zoneMatches(const char *name, const char *input)
        {
            const char *end = name + strlen(name);
            if (sameName(name, end, input))
                return true;
            for (const char *slash = strchr(name, '/'); slash; slash = strchr(slash + 1, '/'))
            {
                const char *part = slash + 1;
                if (sameName(part, end, input))
                    return true;
                const char *next = strchr(part, '/');
                if (next && sameName(part, next, input))
                    return true;
            }
            return false;
        }

        const char *lookup(const char *input)
        {
            for (const Zone &z : kZones)
            {
                if (zoneMatches(z.name, input))
                    return z.posix;
            }
            for (const Alias &a : kAliases)
            {
                if (sameName(a.alias, a.alias + strlen(a.alias), input))
                    return lookup(a.zone);
            }
            return nullptr;
        }

        bool looksLikePosix(const char *s)
        {
            // A POSIX TZ always carries an offset digit (e.g. "UTC0", "<-03>3").
            for (const char *p = s; *p; ++p)
            {
                if (isdigit(static_cast<unsigned char>(*p)))
                    return true;
            }
            return false;
        }
    }

    bool isAuto(const char *setting)
    {
        if (!setting || !*setting)
            return true;
        const char *kAuto = "auto";
        return sameName(kAuto, kAuto + 4, setting);
    }

    const char *zoneName(size_t i)
    {
        return i < sizeof(kZones) / sizeof(kZones[0]) ? kZones[i].name : nullptr;
    }

    const char *toPosix(const char *setting)
    {
        if (isAuto(setting))
            return nullptr;

        if (const char *posix = lookup(setting))
            return posix;

        if (!strchr(setting, '/') && looksLikePosix(setting))
            return setting;

        return nullptr;
    }

    void fromUtcOffset(int32_t offsetSeconds, char *out, size_t size)
    {
        if (offsetSeconds == 0)
        {
            snprintf(out, size, "UTC0");
            return;
        }
        const bool west = offsetSeconds < 0;
        const uint32_t abs = static_cast<uint32_t>(west ? -offsetSeconds : offsetSeconds);
        const unsigned hours = abs / 3600;
        const unsigned minutes = (abs % 3600) / 60;
        // POSIX counts the other way round: zones west of UTC are positive.
        if (minutes)
            snprintf(out, size, "<%c%02u%02u>%s%u:%02u", west ? '-' : '+', hours, minutes,
                     west ? "" : "-", hours, minutes);
        else
            snprintf(out, size, "<%c%02u>%s%u", west ? '-' : '+', hours, west ? "" : "-", hours);
    }
}
