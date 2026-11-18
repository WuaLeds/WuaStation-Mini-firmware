#pragma once

#include <Arduino.h>

// =============================================================================
//  Host -- what the firmware core needs from the system it runs on.
//
//  The core (screens, data providers, settings) does not drive WiFi, the web
//  server or Home Assistant: ESPHome does, and the wuastation component
//  (components/wuastation) implements this interface on top of it.
// =============================================================================
class Host
{
public:
    enum class NetState : uint8_t
    {
        Connecting, // joining the saved network
        Connected,
        Offline, // the saved network has been out of reach for a while
        Setup    // first setup: no network saved, setup access point open
    };

    virtual ~Host() = default;

    virtual NetState netState() const = 0;
    bool isConnected() const { return netState() == NetState::Connected; }
    bool setupActive() const { return netState() == NetState::Setup; }

    // LAN address ("192.168.1.57"), or the access point's while in setup.
    virtual void ip(char *out, size_t size) const = 0;
    // mDNS name without ".local" ("wuastation-mini-3f7590").
    virtual const char *hostname() const = 0;
    virtual void ssid(char *out, size_t size) const = 0;
    virtual int8_t rssi() const = 0;

    // Setup access point. Open for the first setup, and also while the saved
    // network is out of reach (to move the station to a new network).
    virtual bool apActive() const = 0;
    virtual const char *apName() const = 0;

    // A Home Assistant (ESPHome API) client is connected.
    virtual bool homeAssistantConnected() const = 0;

    virtual void restart() = 0;
    // Forget the WiFi network and restart into the setup access point.
    virtual void forgetWifi() = 0;
    // Erase what the host stores (WiFi credentials...). Factory reset.
    virtual void eraseHostData() = 0;
};
