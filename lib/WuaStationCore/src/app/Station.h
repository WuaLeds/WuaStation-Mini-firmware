#pragma once

#include <Arduino.h>

#include "app/Host.h"
#include "config/ScreenConfig.h"
#include "config/Theme.h"
#include "core/Settings.h"
#include "display/Compositor.h"
#include "display/Display.h"
#include "input/Button.h"
#include "net/NetWorker.h"
#include "net/TimeService.h"
#include "providers/sensors/SensorManager.h"
#include "providers/ticker/BinanceTickerProvider.h"
#include "providers/weather/OpenMeteoWeatherProvider.h"
#include "states/AlarmStateProvider.h"
#include "states/AlertStateProvider.h"
#include "states/DateStateProvider.h"
#include "states/DisplayController.h"
#include "states/ForecastStateProvider.h"
#include "states/IndoorStateProvider.h"
#include "states/MessageStateProvider.h"
#include "states/MonthlyCelebrationStateProvider.h"
#include "states/PortalStateProvider.h"
#include "states/SplashStateProvider.h"
#include "states/StatusStateProvider.h"
#include "states/TickerStateProvider.h"
#include "states/TimeStateProvider.h"
#include "states/TimerStateProvider.h"
#include "states/WeatherStateProvider.h"

// =============================================================================
//  Station -- composition root of the WuaStation-mini firmware.
//
//  Owns every data provider and screen, wires them together and maps the
//  buttons to actions. WiFi, the web server and Home Assistant belong to the
//  host (ESPHome, see app/Host.h); the station works the same with or without
//  Home Assistant.
//
//    UP    click : next screen          long : WiFi info (network, address)
//    DOWN  click : brightness step      long : device info (IP, version)
//    TOUCH click : next screen          long : device info
//    any click while an alert/alarm is on screen dismisses it
//    on the timer screen DOWN drives the timer instead (see TimerStateProvider)
//    hold DOWN while powering on : factory reset
//
//  Which screens rotate, in which order and for how long comes from the
//  settings (config/ScreenConfig.h). Every public method runs on the main loop.
// =============================================================================
class Station
{
public:
    // Changes the host mirrors to Home Assistant.
    class Listener
    {
    public:
        virtual ~Listener() = default;
        virtual void onSettingsChanged() {}
        virtual void onButton(ButtonId id, ButtonEvent event)
        {
            (void)id;
            (void)event;
        }
        virtual void onScreenChanged() {}
    };

    explicit Station(Host &host);

    void setup();
    void loop();
    void setListener(Listener *listener) { _listener = listener; }

    // ---- state -------------------------------------------------------------
    const SettingsStore &settings() const { return _settings; }
    const SensorManager &sensors() const { return _sensors; }
    const OpenMeteoWeatherProvider &weather() const { return _weather; }
    const TimeService &time() const { return _time; }
    bool displayOn() const { return _displayOn; }
    // Rotation screen on display (false during system and interrupt screens).
    bool currentScreen(ScreenId &out) const;

    // ---- actions -----------------------------------------------------------
    void setBrightness(uint8_t level); // also turns the display on
    void setDisplayOn(bool on);
    void setMessage(const char *text);
    void setScreenEnabled(ScreenId id, bool enabled);
    void showScreen(ScreenId id);
    void nextScreen();
    // Scrolling notification over any screen (queued).
    void notify(const char *text, Rgb color = Theme::kInfo, uint8_t loops = 1);
    // Every setting at once (settings page). Live where possible; returns
    // true when the change needs a restart, which is then scheduled.
    bool applySettings(const Settings &settings);
    void restartSoon(uint32_t delayMs);
    void factoryReset();

private:
    void checkFactoryReset();
    void registerStates();
    void rebuildRotation();
    IDisplayStateProvider *screen(ScreenId id);
    void handleInput();
    void handleButton(ButtonId id, ButtonEvent event);
    void handleSystemEvents();
    void stepBrightness();
    void syncBrightnessStep(uint8_t level);
    void showInfo();
    void showWifiInfo();
    void settingsChanged();

    Host &_host;
    Listener *_listener = nullptr;

    // ---- services (declaration order = construction order) ----
    WuaDisplay _display;
    Compositor _compositor;
    SettingsStore _settings;
    TimeService _time;
    NetWorker _net;
    BinanceTickerProvider _ticker;
    OpenMeteoWeatherProvider _weather;
    SensorManager _sensors;
    DisplayController _controller;

    // ---- inputs ----
    Button _btnUp;
    Button _btnDown;
    Button _btnTouch;

    // ---- screens (rotation order comes from the settings) ----
    SplashStateProvider _splashState;
    StatusStateProvider _statusState;
    TimeStateProvider _timeState;
    DateStateProvider _dateState;
    WeatherStateProvider _weatherState;
    ForecastStateProvider _forecastState;
    IndoorStateProvider _indoorState;
    TickerStateProvider _tickerState;
    MessageStateProvider _messageState;
    MonthlyCelebrationStateProvider _celebrationState;
    TimerStateProvider _timerState;
    // interrupts
    PortalStateProvider _portalState;
    AlertStateProvider _alertState;
    AlarmStateProvider _alarmState;

    // ---- runtime ----
    bool _displayOn = true;
    uint8_t _brightnessIndex = 0;
    uint32_t _restartAtMs = 0;
    uint32_t _lastTickerCheckMs = 0;
    uint32_t _weatherZoneVersion = 0;
    uint8_t _reportedInvalidTickers = 0; // bit mask
    const IDisplayStateProvider *_lastShown = nullptr;
};
