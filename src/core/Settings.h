#pragma once

#include <Arduino.h>
#include <Preferences.h>

// =============================================================================
//  User settings, persisted in NVS and edited from the settings page
//  (web/SettingsServer).
//  Defaults come from config/Config.h.
// =============================================================================
struct Settings
{
    char timezone[48];  // "auto", zone name ("Buenos Aires", "Europe/Madrid") or POSIX TZ
    char city[48];      // weather location, "City" or "City,CC"
    char latitude[16];  // optional, overrides the city when both are set
    char longitude[16];
    char tickers[64];   // Binance symbols, comma separated ("BTCUSDT,ETHUSDT")
    char alarm[8];      // "HH:MM", empty = disabled
    char message[128];  // custom scrolling message, empty = hidden
    char screens[160];  // rotation layout, see config/ScreenConfig.h
    uint8_t brightness; // 1..255
    float tempOffsetC;  // added to the sensor temperature
    bool europeanAqi;   // air quality: European AQI instead of US AQI
};

class SettingsStore
{
public:
    void begin();

    Settings &get() { return _settings; }
    const Settings &get() const { return _settings; }

    // Normalize and persist every field.
    bool save();

    // Brightness changes (buttons, settings page slider) are written lazily
    // to spare flash.
    void setBrightnessDeferred(uint8_t level);
    void loop();

    // Erase everything (back to defaults on next boot).
    void factoryReset();

    // Parsed helpers.
    bool alarmTime(uint8_t &hour, uint8_t &minute) const;
    bool coordinates(float &latitude, float &longitude) const;

    // Timezone "auto": last POSIX zone reported for the weather location,
    // used at boot until the first forecast arrives. Written only on change.
    const char *locationZone() const { return _locationZone; }
    void rememberLocationZone(const char *posix);

    static void sanitize(Settings &s);
    static void loadDefaults(Settings &s);

private:
    void readString(const char *key, char *dst, size_t size, const char *fallback);

    Preferences _prefs;
    Settings _settings{};
    char _locationZone[64] = {};
    bool _brightnessPending = false;
    uint32_t _brightnessChangedMs = 0;
};
