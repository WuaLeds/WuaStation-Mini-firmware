#include "net/Timezones.h"

#include <string.h>
#include <strings.h>
#include <ctype.h>

namespace Timezones
{
    namespace
    {
        struct Zone
        {
            const char *name;
            const char *posix;
        };

        // Add entries as needed; any POSIX string also works directly.
        const Zone kZones[] = {
            {"UTC", "UTC0"},
            {"Etc/UTC", "UTC0"},
            // Europe
            {"Europe/Madrid", "CET-1CEST,M3.5.0,M10.5.0/3"},
            {"Atlantic/Canary", "WET0WEST,M3.5.0/1,M10.5.0"},
            {"Europe/Lisbon", "WET0WEST,M3.5.0/1,M10.5.0"},
            {"Europe/London", "GMT0BST,M3.5.0/1,M10.5.0"},
            {"Europe/Dublin", "GMT0IST,M3.5.0/1,M10.5.0"},
            {"Europe/Paris", "CET-1CEST,M3.5.0,M10.5.0/3"},
            {"Europe/Berlin", "CET-1CEST,M3.5.0,M10.5.0/3"},
            {"Europe/Rome", "CET-1CEST,M3.5.0,M10.5.0/3"},
            {"Europe/Amsterdam", "CET-1CEST,M3.5.0,M10.5.0/3"},
            {"Europe/Brussels", "CET-1CEST,M3.5.0,M10.5.0/3"},
            {"Europe/Zurich", "CET-1CEST,M3.5.0,M10.5.0/3"},
            {"Europe/Athens", "EET-2EEST,M3.5.0/3,M10.5.0/4"},
            // Americas
            {"America/Argentina/Buenos_Aires", "<-03>3"},
            {"America/Buenos_Aires", "<-03>3"},
            {"America/Montevideo", "<-03>3"},
            {"America/Sao_Paulo", "<-03>3"},
            {"America/Santiago", "<-04>4<-03>,M9.1.6/24,M4.1.6/24"},
            {"America/Asuncion", "<-03>3"},
            {"America/La_Paz", "<-04>4"},
            {"America/Caracas", "<-04>4"},
            {"America/Bogota", "<-05>5"},
            {"America/Lima", "<-05>5"},
            {"America/Guayaquil", "<-05>5"},
            {"America/Mexico_City", "CST6"},
            {"America/New_York", "EST5EDT,M3.2.0,M11.1.0"},
            {"America/Chicago", "CST6CDT,M3.2.0,M11.1.0"},
            {"America/Denver", "MST7MDT,M3.2.0,M11.1.0"},
            {"America/Los_Angeles", "PST8PDT,M3.2.0,M11.1.0"},
            // Asia / Oceania
            {"Asia/Tokyo", "JST-9"},
            {"Asia/Shanghai", "CST-8"},
            {"Asia/Kolkata", "IST-5:30"},
            {"Asia/Dubai", "<+04>-4"},
            {"Australia/Sydney", "AEST-10AEDT,M10.1.0,M4.1.0/3"},
        };

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

    const char *toPosix(const char *setting)
    {
        if (!setting || !*setting)
            return nullptr;

        for (const Zone &z : kZones)
        {
            if (strcasecmp(z.name, setting) == 0)
                return z.posix;
        }

        if (!strchr(setting, '/') && looksLikePosix(setting))
            return setting;

        return nullptr;
    }
}
