#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WebServer.h>
#include <functional>

#include "core/Settings.h"
#include "net/TimeService.h"
#include "net/WifiService.h"
#include "providers/sensors/SensorManager.h"

// =============================================================================
//  SettingsServer -- the Wualeds-branded settings page and its JSON API.
//
//  Lives on WiFiManager's web server (captive portal and LAN), registered
//  before WiFiManager's own routes so "/" is ours; WiFi itself is still set
//  up on WiFiManager's /wifi page.
//
//    GET  /                 settings page (web/settings.html, gzipped)
//    GET  /api/settings     every setting + device status
//    GET  /api/status       device status only (polled by the page)
//    POST /api/settings     JSON body: settings to save (then restart)
//    POST /api/brightness   value=1..255, applied live, saved lazily
//    POST /api/restart
//
//  Handlers run in the main loop (WiFiManager::process()), so they can use
//  the settings and services directly.
// =============================================================================
class SettingsServer
{
public:
    struct Hooks
    {
        std::function<void()> onSaved;               // settings persisted
        std::function<void(uint8_t)> onBrightness;   // live brightness
        std::function<void()> onRestart;
    };

    SettingsServer(SettingsStore &settings, WifiService &wifi, const TimeService &time,
                   const SensorManager &sensors);

    // Call before WifiService::begin().
    void begin(Hooks hooks);

private:
    void registerRoutes(WebServer &server);

    void sendPage(WebServer &server);
    void sendSettings(WebServer &server);
    void sendStatus(WebServer &server);
    void saveSettings(WebServer &server);
    void setBrightness(WebServer &server);
    void restart(WebServer &server);

    void writeStatus(JsonObject out) const;
    static void sendJson(WebServer &server, int code, const JsonDocument &doc);
    static void sendError(WebServer &server, int code, const char *message);

    SettingsStore &_settings;
    WifiService &_wifi;
    const TimeService &_time;
    const SensorManager &_sensors;
    Hooks _hooks;
};
