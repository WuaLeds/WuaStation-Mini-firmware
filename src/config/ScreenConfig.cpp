#include "config/ScreenConfig.h"

#include <ctype.h>
#include <strings.h>
#include "config/Config.h"

namespace ScreenConfig
{
    namespace
    {
        // Same order as ScreenId.
        const char *const kNames[kMaxSlots] = {
            "time", "date", "weather", "forecast", "indoor", "ticker", "message", "celebration", "timer",
        };

        bool lookup(const char *token, size_t len, ScreenId &out)
        {
            for (uint8_t i = 0; i < kMaxSlots; ++i)
            {
                if (strlen(kNames[i]) == len && strncasecmp(kNames[i], token, len) == 0)
                {
                    out = static_cast<ScreenId>(i);
                    return true;
                }
            }
            return false;
        }
    }

    const char *name(ScreenId id)
    {
        const uint8_t i = static_cast<uint8_t>(id);
        return i < kMaxSlots ? kNames[i] : "";
    }

    uint32_t defaultMs(ScreenId id)
    {
        switch (id)
        {
        case ScreenId::Time: return Config::kTimeScreenMs;
        case ScreenId::Date: return Config::kDateScreenMs;
        case ScreenId::Weather: return Config::kWeatherScreenMs;
        case ScreenId::Forecast: return Config::kForecastScreenMs;
        case ScreenId::Indoor: return Config::kIndoorScreenMs;
        case ScreenId::Ticker: return Config::kTickerSymbolMs;
        case ScreenId::Timer: return Config::kTimerScreenMs;
        case ScreenId::Message:
        case ScreenId::Celebration:
        case ScreenId::Count: break;
        }
        return 0;
    }

    bool perSymbol(ScreenId id)
    {
        return id == ScreenId::Ticker;
    }

    uint8_t parse(const char *spec, ScreenSlot *out, uint8_t max)
    {
        uint8_t count = 0;
        bool used[kMaxSlots] = {};
        const char *p = spec;

        while (*p && count < max)
        {
            // One "[-]name[:seconds]" token, surrounding spaces ignored.
            while (*p == ',' || isspace(static_cast<unsigned char>(*p)))
                ++p;
            const bool enabled = *p != '-';
            if (!enabled)
                ++p;
            const char *start = p;
            while (*p && *p != ',' && *p != ':' && !isspace(static_cast<unsigned char>(*p)))
                ++p;
            const size_t len = static_cast<size_t>(p - start);
            while (isspace(static_cast<unsigned char>(*p)))
                ++p;

            unsigned long seconds = 0;
            if (*p == ':')
            {
                ++p;
                char *end = nullptr;
                seconds = strtoul(p, &end, 10);
                p = end;
            }
            while (*p && *p != ',')
                ++p; // skip anything malformed up to the next comma

            ScreenId id;
            if (len == 0 || !lookup(start, len, id))
                continue;
            const uint8_t index = static_cast<uint8_t>(id);
            if (used[index])
                continue;
            used[index] = true;

            if (seconds > Config::kMaxScreenSeconds)
                seconds = Config::kMaxScreenSeconds;
            out[count++] = {id, static_cast<uint16_t>(seconds), enabled};
        }
        return count;
    }

    uint8_t complete(const char *spec, ScreenSlot *out)
    {
        uint8_t count = parse(spec, out);
        for (uint8_t i = 0; i < kMaxSlots; ++i)
        {
            bool listed = false;
            for (uint8_t j = 0; j < count && !listed; ++j)
                listed = static_cast<uint8_t>(out[j].id) == i;
            if (!listed)
                out[count++] = {static_cast<ScreenId>(i), 0, false};
        }
        return count;
    }

    void normalize(char *spec, size_t size)
    {
        ScreenSlot slots[kMaxSlots];
        uint8_t count = parse(spec, slots);
        bool anyEnabled = false;
        for (uint8_t i = 0; i < count; ++i)
            anyEnabled |= slots[i].enabled;
        if (!anyEnabled)
            count = parse(Config::kDefaultScreens, slots);

        size_t pos = 0;
        spec[0] = '\0';
        for (uint8_t i = 0; i < count && pos < size; ++i)
        {
            const char *sep = i ? "," : "";
            const char *flag = slots[i].enabled ? "" : "-";
            const int written = slots[i].seconds > 0
                                    ? snprintf(spec + pos, size - pos, "%s%s%s:%u", sep, flag, name(slots[i].id),
                                               static_cast<unsigned>(slots[i].seconds))
                                    : snprintf(spec + pos, size - pos, "%s%s%s", sep, flag, name(slots[i].id));
            if (written < 0 || static_cast<size_t>(written) >= size - pos)
            {
                spec[pos] = '\0'; // drop the entry that did not fit whole
                break;
            }
            pos += static_cast<size_t>(written);
        }
    }
}
