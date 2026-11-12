#pragma once

#include <Arduino.h>
#include <Preferences.h>

// =============================================================================
//  User settings, persisted in NVS and edited from the WiFi portal.
//  Defaults come from config/Config.h.
// =============================================================================
struct Settings
{
    char timezone[48];  // Olson name ("Europe/Madrid") or POSIX TZ string
    char city[48];      // weather location, "City" or "City,CC"
    char latitude[16];  // optional, overrides the city when both are set
    char longitude[16];
    char tickers[64];   // Binance symbols, comma separated ("BTCUSDT,ETHUSDT")
    char alarm[8];      // "HH:MM", empty = disabled
    char message[128];  // custom scrolling message, empty = hidden
    uint8_t brightness; // 1..255
};

class SettingsStore
{
public:
    void begin();

    Settings &get() { return _settings; }
    const Settings &get() const { return _settings; }

    // Normalize and persist every field.
    bool save();

    // Brightness changes from the buttons are written lazily to spare flash.
    void setBrightnessDeferred(uint8_t level);
    void loop();

    // Erase everything (back to defaults on next boot).
    void factoryReset();

    // Parsed helpers.
    bool alarmTime(uint8_t &hour, uint8_t &minute) const;
    bool coordinates(float &latitude, float &longitude) const;

    static void sanitize(Settings &s);
    static void loadDefaults(Settings &s);

private:
    void readString(const char *key, char *dst, size_t size, const char *fallback);

    Preferences _prefs;
    Settings _settings{};
    bool _brightnessPending = false;
    uint32_t _brightnessChangedMs = 0;
};
