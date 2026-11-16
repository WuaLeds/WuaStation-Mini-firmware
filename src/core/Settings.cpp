#include "core/Settings.h"

#include <ctype.h>
#include <math.h>
#include "config/Config.h"
#include "config/ScreenConfig.h"
#include "core/Logger.h"

namespace
{
    constexpr const char *kNamespace = "wuastation";

    // NVS keys (max 15 chars)
    constexpr const char *kKeyTimezone = "tz";
    constexpr const char *kKeyCity = "city";
    constexpr const char *kKeyLatitude = "lat";
    constexpr const char *kKeyLongitude = "lon";
    constexpr const char *kKeyTickers = "tickers";
    constexpr const char *kKeyAlarm = "alarm";
    constexpr const char *kKeyMessage = "msg";
    constexpr const char *kKeyScreens = "screens";
    constexpr const char *kKeyBrightness = "bright";
    constexpr const char *kKeyTempOffset = "toff";
    constexpr const char *kKeyEuropeanAqi = "aqeu";
    constexpr const char *kKeyLocationZone = "tzauto"; // not a user setting

    // Trim leading/trailing whitespace in place.
    void trim(char *s)
    {
        char *start = s;
        while (*start && isspace(static_cast<unsigned char>(*start)))
            ++start;
        if (start != s)
            memmove(s, start, strlen(start) + 1);
        size_t len = strlen(s);
        while (len > 0 && isspace(static_cast<unsigned char>(s[len - 1])))
            s[--len] = '\0';
    }

    // "btc usdt, eth-usdt" -> "BTCUSDT,ETHUSDT" (alnum and commas only).
    void normalizeTickers(char *s)
    {
        char *out = s;
        for (const char *in = s; *in; ++in)
        {
            const char c = *in;
            if (isalnum(static_cast<unsigned char>(c)))
                *out++ = static_cast<char>(toupper(static_cast<unsigned char>(c)));
            else if (c == ',' && out != s && out[-1] != ',')
                *out++ = ',';
        }
        if (out != s && out[-1] == ',')
            --out;
        *out = '\0';
    }

    bool parseAlarm(const char *s, uint8_t &h, uint8_t &m)
    {
        unsigned hh = 0, mm = 0;
        char extra = 0;
        if (sscanf(s, "%u:%u%c", &hh, &mm, &extra) != 2)
            return false;
        if (hh > 23 || mm > 59)
            return false;
        h = static_cast<uint8_t>(hh);
        m = static_cast<uint8_t>(mm);
        return true;
    }

    bool parseFloat(const char *s, float &out)
    {
        if (!s || !*s)
            return false;
        char *end = nullptr;
        const float v = strtof(s, &end);
        if (end == s)
            return false;
        out = v;
        return true;
    }
}

void SettingsStore::loadDefaults(Settings &s)
{
    memset(&s, 0, sizeof(s));
    strlcpy(s.timezone, Config::kDefaultTimezone, sizeof(s.timezone));
    strlcpy(s.city, Config::kDefaultCity, sizeof(s.city));
    strlcpy(s.tickers, Config::kDefaultTickers, sizeof(s.tickers));
    strlcpy(s.screens, Config::kDefaultScreens, sizeof(s.screens));
    s.brightness = Config::kDefaultBrightness;
    s.tempOffsetC = Config::kSensorTempOffsetC;
    s.europeanAqi = Config::kAirQualityEuropean;
}

void SettingsStore::sanitize(Settings &s)
{
    trim(s.timezone);
    trim(s.city);
    trim(s.latitude);
    trim(s.longitude);
    trim(s.message);
    trim(s.alarm);
    normalizeTickers(s.tickers);
    ScreenConfig::normalize(s.screens, sizeof(s.screens));

    if (s.timezone[0] == '\0')
        strlcpy(s.timezone, Config::kDefaultTimezone, sizeof(s.timezone));

    uint8_t h = 0, m = 0;
    if (s.alarm[0] != '\0')
    {
        if (parseAlarm(s.alarm, h, m))
            snprintf(s.alarm, sizeof(s.alarm), "%02u:%02u",
                     static_cast<unsigned>(h), static_cast<unsigned>(m));
        else
            s.alarm[0] = '\0';
    }

    float lat, lon;
    if (!parseFloat(s.latitude, lat) || !parseFloat(s.longitude, lon) ||
        lat < -90.0f || lat > 90.0f || lon < -180.0f || lon > 180.0f)
    {
        s.latitude[0] = '\0';
        s.longitude[0] = '\0';
    }

    if (s.brightness == 0)
        s.brightness = 1;

    if (!(s.tempOffsetC >= -Config::kSensorTempOffsetMaxC && s.tempOffsetC <= Config::kSensorTempOffsetMaxC))
        s.tempOffsetC = 0.0f; // also catches NaN
    s.tempOffsetC = roundf(s.tempOffsetC * 10.0f) / 10.0f;
}

void SettingsStore::readString(const char *key, char *dst, size_t size, const char *fallback)
{
    // isKey() first: getString() on a missing key logs an NVS error.
    if (_prefs.isKey(key))
        _prefs.getString(key, dst, size);
    else
        strlcpy(dst, fallback, size);
}

void SettingsStore::begin()
{
    Settings defaults;
    loadDefaults(defaults);

    _prefs.begin(kNamespace, false);
    readString(kKeyTimezone, _settings.timezone, sizeof(_settings.timezone), defaults.timezone);
    readString(kKeyCity, _settings.city, sizeof(_settings.city), defaults.city);
    readString(kKeyLatitude, _settings.latitude, sizeof(_settings.latitude), defaults.latitude);
    readString(kKeyLongitude, _settings.longitude, sizeof(_settings.longitude), defaults.longitude);
    readString(kKeyTickers, _settings.tickers, sizeof(_settings.tickers), defaults.tickers);
    readString(kKeyAlarm, _settings.alarm, sizeof(_settings.alarm), defaults.alarm);
    readString(kKeyMessage, _settings.message, sizeof(_settings.message), defaults.message);
    readString(kKeyScreens, _settings.screens, sizeof(_settings.screens), defaults.screens);
    _settings.brightness = _prefs.getUChar(kKeyBrightness, defaults.brightness);
    _settings.tempOffsetC = _prefs.isKey(kKeyTempOffset) ? _prefs.getFloat(kKeyTempOffset) : defaults.tempOffsetC;
    _settings.europeanAqi = _prefs.getBool(kKeyEuropeanAqi, defaults.europeanAqi);
    readString(kKeyLocationZone, _locationZone, sizeof(_locationZone), "");

    sanitize(_settings);

    LOG_INFO("Settings: tz=%s city=%s lat=%s lon=%s tickers=%s alarm=%s bright=%u",
             _settings.timezone, _settings.city, _settings.latitude, _settings.longitude,
             _settings.tickers, _settings.alarm, _settings.brightness);
    LOG_INFO("Screens: %s | sensor offset %.1f C | %s AQI", _settings.screens, _settings.tempOffsetC,
             _settings.europeanAqi ? "European" : "US");
    if (_locationZone[0])
        LOG_INFO("Last location zone: %s", _locationZone);
}

bool SettingsStore::save()
{
    sanitize(_settings);
    bool ok = true;
    ok &= _prefs.putString(kKeyTimezone, _settings.timezone) > 0;
    // Optional fields may legitimately be empty (putString returns 0 then).
    _prefs.putString(kKeyCity, _settings.city);
    _prefs.putString(kKeyLatitude, _settings.latitude);
    _prefs.putString(kKeyLongitude, _settings.longitude);
    _prefs.putString(kKeyTickers, _settings.tickers);
    _prefs.putString(kKeyAlarm, _settings.alarm);
    _prefs.putString(kKeyMessage, _settings.message);
    ok &= _prefs.putString(kKeyScreens, _settings.screens) > 0;
    ok &= _prefs.putUChar(kKeyBrightness, _settings.brightness) > 0;
    ok &= _prefs.putFloat(kKeyTempOffset, _settings.tempOffsetC) > 0;
    ok &= _prefs.putBool(kKeyEuropeanAqi, _settings.europeanAqi) > 0;
    _brightnessPending = false;
    LOG_INFO("Settings saved (%s)", ok ? "ok" : "error");
    return ok;
}

void SettingsStore::setBrightnessDeferred(uint8_t level)
{
    _settings.brightness = level == 0 ? 1 : level;
    _brightnessPending = true;
    _brightnessChangedMs = millis();
}

void SettingsStore::loop()
{
    if (_brightnessPending && millis() - _brightnessChangedMs >= Config::kBrightnessSaveDelayMs)
    {
        _brightnessPending = false;
        _prefs.putUChar(kKeyBrightness, _settings.brightness);
        LOG_DEBUG("Brightness %u stored", _settings.brightness);
    }
}

void SettingsStore::factoryReset()
{
    _prefs.clear();
    loadDefaults(_settings);
    _locationZone[0] = '\0';
    LOG_WARN("Settings erased");
}

void SettingsStore::rememberLocationZone(const char *posix)
{
    if (!posix || strcmp(posix, _locationZone) == 0)
        return;
    strlcpy(_locationZone, posix, sizeof(_locationZone));
    _prefs.putString(kKeyLocationZone, _locationZone);
    LOG_DEBUG("Location zone %s stored", _locationZone);
}

bool SettingsStore::alarmTime(uint8_t &hour, uint8_t &minute) const
{
    return _settings.alarm[0] != '\0' && parseAlarm(_settings.alarm, hour, minute);
}

bool SettingsStore::coordinates(float &latitude, float &longitude) const
{
    return parseFloat(_settings.latitude, latitude) && parseFloat(_settings.longitude, longitude);
}
