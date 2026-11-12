#pragma once

#include <Arduino.h>

#include "core/Settings.h"
#include "display/Compositor.h"
#include "display/Display.h"
#include "input/Button.h"
#include "net/NetWorker.h"
#include "net/TimeService.h"
#include "net/WifiService.h"
#include "providers/sensors/SensorManager.h"
#include "providers/ticker/BinanceTickerProvider.h"
#include "providers/weather/OpenMeteoWeatherProvider.h"
#include "states/AlarmStateProvider.h"
#include "states/AlertStateProvider.h"
#include "states/DateStateProvider.h"
#include "states/DisplayController.h"
#include "states/IndoorStateProvider.h"
#include "states/MessageStateProvider.h"
#include "states/MonthlyCelebrationStateProvider.h"
#include "states/PortalStateProvider.h"
#include "states/SplashStateProvider.h"
#include "states/StatusStateProvider.h"
#include "states/TickerStateProvider.h"
#include "states/TimeStateProvider.h"
#include "states/WeatherStateProvider.h"

// =============================================================================
//  App -- composition root of the WuaStation-mini firmware.
//
//  Owns every service, data provider and screen, wires them together and
//  maps the buttons to actions:
//
//    UP    click : next screen          long : WiFi setup portal (on/off)
//    DOWN  click : brightness step      long : device info (IP...)
//    TOUCH click : next screen          long : device info
//    any click while an alert/alarm is on screen dismisses it
//    hold DOWN while powering on : factory reset
// =============================================================================
class App
{
public:
    App();

    void setup();
    void loop();

private:
    void checkFactoryReset();
    void registerStates();
    void handleInput();
    void handleButton(ButtonId id, ButtonEvent event);
    void handleSystemEvents();
    void stepBrightness();
    void showInfo();
    void onSettingsSaved();

    // ---- services (declaration order = construction order) ----
    WuaDisplay _display;
    Compositor _compositor;
    SettingsStore _settings;
    WifiService _wifi;
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

    // ---- screens (rotation order = registration order in registerStates) ----
    SplashStateProvider _splashState;
    StatusStateProvider _statusState;
    TimeStateProvider _timeState;
    DateStateProvider _dateState;
    WeatherStateProvider _weatherState;
    IndoorStateProvider _indoorState;
    TickerStateProvider _tickerState;
    MessageStateProvider _messageState;
    MonthlyCelebrationStateProvider _celebrationState;
    // interrupts
    PortalStateProvider _portalState;
    AlertStateProvider _alertState;
    AlarmStateProvider _alarmState;

    // ---- runtime ----
    uint8_t _brightnessIndex = 0;
    uint32_t _restartAtMs = 0;
    uint32_t _lastTickerCheckMs = 0;
    uint8_t _reportedInvalidTickers = 0; // bit mask
};
