#include "web/SettingsJson.h"

#include <math.h>
#include <time.h>
#include "config/Config.h"
#include "config/ScreenConfig.h"
#include "net/Timezones.h"

namespace
{
    // strlcpy() that never leaves half a UTF-8 character at the end.
    void copyUtf8(char *dst, const char *src, size_t size)
    {
        strlcpy(dst, src, size);
        if (strlen(src) < size)
            return; // nothing was cut
        size_t n = strlen(dst);
        if (n == 0)
            return;
        size_t start = n - 1;
        while (start > 0 && (static_cast<uint8_t>(dst[start]) & 0xC0) == 0x80)
            --start;
        const uint8_t lead = static_cast<uint8_t>(dst[start]);
        const size_t length = lead >= 0xF0 ? 4 : lead >= 0xE0 ? 3 : lead >= 0xC0 ? 2 : 1;
        if (start + length > n)
            dst[start] = '\0';
    }

    void copyField(JsonObjectConst in, const char *key, char *dst, size_t size)
    {
        JsonVariantConst value = in[key];
        if (value.is<const char *>())
            copyUtf8(dst, value.as<const char *>(), size);
    }
}

namespace SettingsJson
{
    void writeStatus(JsonObject out, const Host &host, const TimeService &time, const SensorManager &sensors)
    {
        char ip[16] = "";
        char ssid[33] = "";
        host.ip(ip, sizeof(ip));
        host.ssid(ssid, sizeof(ssid));

        out["version"] = WUA_FIRMWARE_VERSION;
        out["hostname"] = host.hostname();
        out["connected"] = host.isConnected();
        out["portal"] = host.setupActive();
        out["ip"] = ip;
        out["ssid"] = ssid;
        out["rssi"] = host.rssi();
        out["homeAssistant"] = host.homeAssistantConnected();
        out["apName"] = host.apName();
        out["apPassword"] = host.apPassword();
        out["sensor"] = sensors.sensorName();
        if (sensors.hasReading())
        {
            const SensorReading &r = sensors.reading();
            out["tempC"] = roundf(r.temperatureC * 10.0f) / 10.0f;
            if (r.hasHumidity)
                out["humidity"] = static_cast<int>(lroundf(r.humidity));
        }

        char clock[6] = "";
        struct tm now;
        if (time.localTime(now))
            strftime(clock, sizeof(clock), "%H:%M", &now);
        out["clock"] = clock;
        out["uptime"] = millis() / 1000UL;
    }

    void writeSettings(JsonObject out, const Settings &s)
    {
        out["tz"] = s.timezone;
        out["city"] = s.city;
        out["lat"] = s.latitude;
        out["lon"] = s.longitude;
        out["tickers"] = s.tickers;
        out["alarm"] = s.alarm;
        out["msg"] = s.message;
        out["bright"] = s.brightness;
        out["tempOffset"] = s.tempOffsetC;
        out["aqEu"] = s.europeanAqi;
        out["maxSeconds"] = Config::kMaxScreenSeconds;
        out["maxTickers"] = Config::kMaxTickers;

        JsonArray zones = out["zones"].to<JsonArray>();
        for (size_t i = 0; const char *zone = Timezones::zoneName(i); ++i)
            zones.add(zone);

        // Every screen, the configured ones first in their order.
        ScreenSlot slots[ScreenConfig::kMaxSlots];
        const uint8_t count = ScreenConfig::complete(s.screens, slots);
        JsonArray screens = out["screens"].to<JsonArray>();
        for (uint8_t i = 0; i < count; ++i)
        {
            JsonObject o = screens.add<JsonObject>();
            o["id"] = ScreenConfig::name(slots[i].id);
            o["on"] = slots[i].enabled;
            o["s"] = slots[i].seconds;
            o["def"] = ScreenConfig::defaultMs(slots[i].id) / 1000.0f;
            o["unit"] = ScreenConfig::perSymbol(slots[i].id) ? "symbol" : "s";
        }
    }

    bool read(JsonVariantConst in, Settings &s)
    {
        if (!in.is<JsonObjectConst>())
            return false;
        JsonObjectConst obj = in.as<JsonObjectConst>();
        copyField(obj, "tz", s.timezone, sizeof(s.timezone));
        copyField(obj, "city", s.city, sizeof(s.city));
        copyField(obj, "lat", s.latitude, sizeof(s.latitude));
        copyField(obj, "lon", s.longitude, sizeof(s.longitude));
        copyField(obj, "tickers", s.tickers, sizeof(s.tickers));
        copyField(obj, "alarm", s.alarm, sizeof(s.alarm));
        copyField(obj, "msg", s.message, sizeof(s.message));
        copyField(obj, "screens", s.screens, sizeof(s.screens));
        if (obj["bright"].is<int>())
            s.brightness = static_cast<uint8_t>(constrain(obj["bright"].as<int>(), 1, 255));
        if (obj["tempOffset"].is<float>())
            s.tempOffsetC = obj["tempOffset"].as<float>();
        if (obj["aqEu"].is<bool>())
            s.europeanAqi = obj["aqEu"].as<bool>();
        return true;
    }
}
