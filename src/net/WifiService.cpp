#include "net/WifiService.h"

#include <ESPmDNS.h>
#include <ctype.h>
#include <vector>
#include "board/Board.h"
#include "config/Config.h"
#include "core/Logger.h"

void WifiService::begin()
{
    snprintf(_apName, sizeof(_apName), "%s-%s", Config::kDeviceName, Board::deviceId());
    snprintf(_hostname, sizeof(_hostname), "%s-%s", Config::kHostnamePrefix, Board::deviceId());
    for (char *p = _hostname; *p; ++p)
        *p = static_cast<char>(tolower(static_cast<unsigned char>(*p)));

    // The hostname must be set before the STA interface starts.
    WiFi.setHostname(_hostname);
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);

#if defined(WUA_ENABLE_DEBUG)
    _wm.setDebugOutput(true);
#else
    _wm.setDebugOutput(false);
#endif
    _wm.setConfigPortalBlocking(false);
    _wm.setConnectTimeout(Config::kWifiConnectTimeoutMs / 1000);
    _wm.setHostname(_hostname);
    _wm.setTitle(Config::kDeviceName);
    _wm.setDarkMode(true);
    std::vector<const char *> menu = {"wifi", "info", "restart"};
    _wm.setMenu(menu);
    // Runs before WiFiManager adds its own routes, so ours win.
    _wm.setWebServerCallback([this]() {
        if (_onWebServer && _wm.server)
            _onWebServer(*_wm.server);
    });

    if (hasCredentials())
        beginConnect();
    else
        startPortal();
}

const char *WifiService::apPassword() const
{
    return Config::kPortalPassword;
}

void WifiService::setState(State s)
{
    _state = s;
    _stateSinceMs = millis();
    static const char *const kNames[] = {"Idle", "Connecting", "Connected", "Offline", "Portal"};
    LOG_INFO("WiFi state: %s", kNames[static_cast<uint8_t>(s)]);
}

void WifiService::beginConnect()
{
    LOG_INFO("Connecting to saved network...");
    WiFi.begin();
    setState(State::Connecting);
}

void WifiService::handleConnected()
{
    setState(State::Connected);
    _everConnected = true;
    LOG_INFO("Connected to '%s', IP %s, RSSI %d dBm", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str(),
             WiFi.RSSI());

    if (!_mdnsStarted && MDNS.begin(_hostname))
    {
        MDNS.addService("http", "tcp", 80);
        _mdnsStarted = true;
    }
    if (Config::kWebPortalOnStation && !_wm.getWebPortalActive())
        _wm.startWebPortal();

    if (_onConnected)
        _onConnected();
}

void WifiService::startPortal()
{
    if (_state == State::Portal)
        return;
    if (_wm.getWebPortalActive())
        _wm.stopWebPortal();

    _portalStartedConnected = isConnected();
    LOG_INFO("Starting config portal '%s'", _apName);
    // Non-blocking mode: returns immediately, served from loop() via process().
    _wm.startConfigPortal(_apName, apPassword());
    setState(State::Portal);
}

void WifiService::stopPortal()
{
    if (_wm.getConfigPortalActive())
        _wm.stopConfigPortal();
}

void WifiService::cancelPortal()
{
    if (_state != State::Portal)
        return;
    if (!isConnected() && !hasCredentials())
        return; // nothing to go back to
    stopPortal();
    if (isConnected())
        handleConnected();
    else
        beginConnect();
}

void WifiService::forgetCredentials()
{
    _wm.resetSettings();
}

void WifiService::loop()
{
    bool savedAndConnected = false;
    if (_wm.getConfigPortalActive() || _wm.getWebPortalActive())
        savedAndConnected = _wm.process();

    const bool up = isConnected();
    const uint32_t now = millis();
    const uint32_t inState = now - _stateSinceMs;

    switch (_state)
    {
    case State::Idle: break;

    case State::Connecting:
        if (up)
            handleConnected();
        else if (_everConnected && inState >= Config::kOfflineAfterMs)
        {
            setState(State::Offline);
            _lastReconnectMs = now;
        }
        else if (!_everConnected && inState >= Config::kWifiConnectTimeoutMs)
            startPortal(); // credentials look wrong or the network is gone
        break;

    case State::Connected:
        if (!up)
            setState(State::Connecting); // the driver reconnects on its own
        break;

    case State::Offline:
        if (up)
            handleConnected();
        else if (now - _lastReconnectMs >= Config::kReconnectIntervalMs)
        {
            _lastReconnectMs = now;
            WiFi.reconnect();
        }
        break;

    case State::Portal:
        if (savedAndConnected || (!_portalStartedConnected && up))
        {
            stopPortal();
            handleConnected();
        }
        else if (!_wm.getConfigPortalActive())
        {
            // Closed by WiFiManager itself.
            if (up)
                handleConnected();
            else
                beginConnect();
        }
        else if (hasCredentials() && inState >= Config::kPortalTimeoutMs)
        {
            LOG_INFO("Config portal timeout");
            stopPortal();
            if (up)
                handleConnected();
            else
                beginConnect();
        }
        break;
    }
}
