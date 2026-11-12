#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <functional>
#include "core/Settings.h"

// =============================================================================
//  WifiService -- non-blocking WiFi life cycle.
//
//    boot ── saved credentials? ──yes──> Connecting ──ok──> Connected
//                │                          │ timeout (never connected)
//                no                         v
//                └──────────────────────> Portal (AP "WuaStation-XXXX")
//
//    Connected ── link lost ──> Connecting ── 30 s ──> Offline (retries)
//
//  The captive portal (WiFiManager, non-blocking) also exposes the user
//  settings. While connected, the same settings page is served on the LAN IP
//  (http://wuastation-xxxx.local) so nothing needs the AP after first setup.
// =============================================================================
class WifiService
{
public:
    enum class State : uint8_t
    {
        Idle,
        Connecting,
        Connected,
        Offline,
        Portal
    };

    using Callback = std::function<void()>;

    void begin(SettingsStore &settings);
    void loop();

    // Open the configuration access point (UP long-press).
    void startPortal();
    // Close it again and go back to the saved network (if any).
    void cancelPortal();
    // Erase the stored WiFi credentials.
    void forgetCredentials();

    State state() const { return _state; }
    bool isConnected() const { return WiFi.status() == WL_CONNECTED; }
    bool portalActive() const { return _state == State::Portal; }
    bool hasCredentials() { return _wm.getWiFiIsSaved(); }

    const char *apName() const { return _apName; }
    const char *apPassword() const;
    const char *hostname() const { return _hostname; }
    IPAddress ip() const { return WiFi.localIP(); }

    // Fired on every (re)connection.
    void onConnected(Callback cb) { _onConnected = std::move(cb); }
    // Fired after the settings page was saved (values already persisted).
    void onSettingsSaved(Callback cb) { _onSettingsSaved = std::move(cb); }

private:
    void setState(State s);
    void beginConnect();
    void handleConnected();
    void stopPortal();
    void setupParameters();
    void applyParameters();

    WiFiManager _wm;
    SettingsStore *_settings = nullptr;

    State _state = State::Idle;
    uint32_t _stateSinceMs = 0;
    uint32_t _lastReconnectMs = 0;
    bool _everConnected = false;
    bool _portalStartedConnected = false;
    bool _mdnsStarted = false;

    char _apName[32] = {};
    char _hostname[32] = {};

    Callback _onConnected;
    Callback _onSettingsSaved;

    // Settings page fields (WiFiManager keeps pointers to them).
    WiFiManagerParameter _pTimezone{"tz", "Timezone (Europe/Madrid, America/Argentina/Buenos_Aires or POSIX TZ)", "", 47};
    WiFiManagerParameter _pCity{"city", "Weather city (Madrid or Madrid,ES)", "", 47};
    WiFiManagerParameter _pLatitude{"lat", "Latitude (optional, overrides the city)", "", 15};
    WiFiManagerParameter _pLongitude{"lon", "Longitude (optional)", "", 15};
    WiFiManagerParameter _pTickers{"tickers", "Binance tickers, comma separated, max 4 (BTCUSDT,ETHUSDT)", "", 63};
    WiFiManagerParameter _pAlarm{"alarm", "Alarm HH:MM (empty = off)", "", 5, "placeholder='07:30'"};
    WiFiManagerParameter _pMessage{"msg", "Custom message (empty = off)", "", 127};
    WiFiManagerParameter _pBrightness{"bright", "Brightness 1-255", "", 3, "type='number' min='1' max='255'"};
};
