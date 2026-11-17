#pragma once

#include <Arduino.h>
#include <Preferences.h>

// =============================================================================
//  User settings, persisted in NVS (namespace "wuastation") and edited from
//  the settings page, the buttons and Home Assistant. Defaults come from
//  config/Config.h.
//
//  Only the main loop writes them. The web server runs in its own task and
//  reads a consistent copy with snapshot().
// =============================================================================
struct Settings
{
    char timezone[48]; // "auto", zone name ("Buenos Aires", "Europe/Madrid") or POSIX TZ
    char city[48];     // weather location, "City" or "City,CC"
    char latitude[16]; // optional, overrides the city when both are set
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

    // Main loop only.
    const Settings &get() const { return _settings; }
    // Any task: consistent copy. snapshotInto() fills caller storage, for
    // tasks with a small stack (the web server's) that keep it on the heap.
    Settings snapshot() const;
    void snapshotInto(Settings &out) const;

    // Normalize, apply and persist every field (main loop).
    bool update(const Settings &settings);

    // Whether going from `before` to `after` needs a restart (data sources
    // are configured once at boot: city, coordinates, tickers).
    static bool needsRestart(const Settings &before, const Settings &after);

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
    static bool parseAlarm(const char *text, uint8_t &hour, uint8_t &minute);

private:
    void readString(const char *key, char *dst, size_t size, const char *fallback);
    bool persist();

    Preferences _prefs;
    Settings _settings{};
    mutable portMUX_TYPE _lock = portMUX_INITIALIZER_UNLOCKED;
    char _locationZone[64] = {};
    bool _brightnessPending = false;
    uint32_t _brightnessChangedMs = 0;
};
