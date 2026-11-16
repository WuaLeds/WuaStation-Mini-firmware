#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <functional>

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
//  WiFiManager (non-blocking) provides the captive portal and its WiFi pages
//  (/wifi). The station's own settings page is registered on the same web
//  server through onWebServer() (see web/SettingsServer), both in the portal
//  and on the LAN IP (http://wuastation-xxxx.local) once connected.
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
    using ServerCallback = std::function<void(WebServer &)>;

    void begin();
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
    // Fired whenever WiFiManager (re)creates its web server, before its own
    // routes: handlers registered here take precedence (e.g. "/").
    // Call before begin().
    void onWebServer(ServerCallback cb) { _onWebServer = std::move(cb); }

    // Name of the WiFi network in use (empty when not connected).
    String ssid() const { return isConnected() ? WiFi.SSID() : String(); }
    int8_t rssi() const { return isConnected() ? WiFi.RSSI() : 0; }

private:
    void setState(State s);
    void beginConnect();
    void handleConnected();
    void stopPortal();

    WiFiManager _wm;

    State _state = State::Idle;
    uint32_t _stateSinceMs = 0;
    uint32_t _lastReconnectMs = 0;
    bool _everConnected = false;
    bool _portalStartedConnected = false;
    bool _mdnsStarted = false;

    char _apName[32] = {};
    char _hostname[32] = {};

    Callback _onConnected;
    ServerCallback _onWebServer;
};
