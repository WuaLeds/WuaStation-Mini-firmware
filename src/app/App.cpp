#include "app/App.h"

#include "board/Board.h"
#include "board/Pins.h"
#include "config/Config.h"
#include "config/Strings.h"
#include "config/Theme.h"
#include "core/Logger.h"

namespace
{
    constexpr uint8_t kBrightnessLevelCount = sizeof(Config::kBrightnessLevels) / sizeof(Config::kBrightnessLevels[0]);

    const CalendarStateProvider::Date kCelebrationDate{
        Config::kCelebrationYear, Config::kCelebrationMonth, Config::kCelebrationDay};
}

App::App()
    : _compositor(_display),
      _controller(_display, _compositor),
      _btnUp(Pins::kButtonUp, false, INPUT_PULLUP),
      _btnDown(Pins::kButtonDown, false, INPUT_PULLUP),
      _btnTouch(Pins::kTouch, true, INPUT),
      _splashState(_compositor),
      _statusState(_compositor, _wifi, _time),
      _timeState(_compositor, _time),
      _dateState(_compositor, _time),
      _weatherState(_compositor, _weather),
      _indoorState(_compositor, _sensors),
      _tickerState(_compositor, _ticker),
      _messageState(_compositor, _settings),
      _celebrationState(_compositor, _time, kCelebrationDate),
      _portalState(_compositor, _wifi),
      _alertState(_compositor),
      _alarmState(_compositor, _time, _settings)
{
}

// -----------------------------------------------------------------------------
//  Setup
// -----------------------------------------------------------------------------
void App::setup()
{
    Serial.begin(115200);
#if defined(WUA_WAIT_FOR_SERIAL)
    while (!Serial)
        delay(10);
#endif
    Board::init();
    LOG_INFO("WuaStation-mini v%s (id %s)", WUA_FIRMWARE_VERSION, Board::deviceId());

    _settings.begin();
    const Settings &s = _settings.get();

    // Display (WuaDisplay-LMX, LMX1p backend)
    _display.begin();
    _display.panel().cp437(true); // real CP437 glyphs (accents, degree sign)
    _display.setBrightness(s.brightness);
    _compositor.begin();

    // Brightness step closest to the stored value
    for (uint8_t i = 0; i < kBrightnessLevelCount; ++i)
    {
        if (Config::kBrightnessLevels[i] <= s.brightness)
            _brightnessIndex = i;
    }

    _btnUp.begin();
    _btnDown.begin();
    _btnTouch.begin();
    checkFactoryReset();

    // Clock and data sources (configured once; settings changes restart)
    _time.begin(s.timezone);
    _ticker.configure(s.tickers);
    float lat = 0.0f, lon = 0.0f;
    const bool hasCoords = _settings.coordinates(lat, lon);
    _weather.configure(s.city, hasCoords, lat, lon);
    _net.add(&_ticker);
    _net.add(&_weather);
    _sensors.begin();

    registerStates();
    _controller.begin();

    // WiFi last: it may open the portal right away
    _wifi.onConnected([this]() { _time.startSync(); });
    _wifi.onSettingsSaved([this]() { onSettingsSaved(); });
    _wifi.begin(_settings);
    _net.begin();

    if (!_time.timezoneKnown())
    {
        char buf[96];
        snprintf(buf, sizeof(buf), Strings::kTimezoneUnknownFmt, s.timezone);
        _alertState.push(buf);
    }

    LOG_INFO("Setup done, free heap %u", ESP.getFreeHeap());
}

void App::registerStates()
{
    // Rotation, in display order. States without data are skipped.
    _controller.addState(&_splashState);
    _controller.addState(&_statusState);
    _controller.addState(&_timeState);
    _controller.addState(&_dateState);
    _controller.addState(&_weatherState);
    _controller.addState(&_indoorState);
    _controller.addState(&_tickerState);
    _controller.addState(&_messageState);
    _controller.addState(&_celebrationState);

    // Interrupt screens (priority > 0).
    _controller.addState(&_portalState);
    _controller.addState(&_alertState);
    _controller.addState(&_alarmState);
}

void App::checkFactoryReset()
{
    if (!_btnDown.isPressedRaw())
        return;

    LOG_WARN("DOWN held at boot: factory reset in %u ms", Config::kFactoryResetHoldMs);
    _display.setTextColorRGB(Theme::kAlert.r, Theme::kAlert.g, Theme::kAlert.b);
    _display.printAlignedRefresh(Strings::kFactoryResetPrompt, WuaDisplay::HAlign::Center, Layout::kTextY);

    const uint32_t start = millis();
    while (_btnDown.isPressedRaw())
    {
        if (millis() - start >= Config::kFactoryResetHoldMs)
        {
            _display.setTextColorRGB(Theme::kInfo.r, Theme::kInfo.g, Theme::kInfo.b);
            _display.printAlignedRefresh(Strings::kFactoryResetDone, WuaDisplay::HAlign::Center, Layout::kTextY);
            _settings.factoryReset();
            _wifi.forgetCredentials();
            delay(1000);
            ESP.restart();
        }
        delay(10);
    }
    // Released early: carry on with a normal boot.
    _display.clear();
    _display.flush();
}

// -----------------------------------------------------------------------------
//  Loop
// -----------------------------------------------------------------------------
void App::loop()
{
    _wifi.loop();
    handleInput();
    _sensors.loop();
    _settings.loop();
    handleSystemEvents();
    _controller.loop();

    if (_restartAtMs != 0 && static_cast<int32_t>(millis() - _restartAtMs) >= 0)
    {
        LOG_INFO("Restarting to apply the new settings");
        delay(100);
        ESP.restart();
    }

    delay(1); // let the idle task run (single-core C3)
}

void App::handleInput()
{
    const ButtonEvent up = _btnUp.poll();
    if (up != ButtonEvent::None)
        handleButton(ButtonId::Up, up);
    const ButtonEvent down = _btnDown.poll();
    if (down != ButtonEvent::None)
        handleButton(ButtonId::Down, down);
    const ButtonEvent touch = _btnTouch.poll();
    if (touch != ButtonEvent::None)
        handleButton(ButtonId::Touch, touch);
}

void App::handleButton(ButtonId id, ButtonEvent event)
{
    LOG_DEBUG("Button %u event %u", static_cast<unsigned>(id), static_cast<unsigned>(event));

    // The screen on display gets the first chance (dismiss alarm/alert...).
    if (_controller.dispatchButton(id, event))
        return;

    switch (id)
    {
    case ButtonId::Up:
        if (event == ButtonEvent::Click)
            _controller.next();
        else if (event == ButtonEvent::LongPress)
            _wifi.portalActive() ? _wifi.cancelPortal() : _wifi.startPortal();
        break;

    case ButtonId::Down:
        if (event == ButtonEvent::Click)
            stepBrightness();
        else if (event == ButtonEvent::LongPress)
            showInfo();
        break;

    case ButtonId::Touch:
        if (event == ButtonEvent::Click)
            _controller.next();
        else if (event == ButtonEvent::LongPress)
            showInfo();
        break;
    }
}

void App::stepBrightness()
{
    _brightnessIndex = (_brightnessIndex + 1) % kBrightnessLevelCount;
    const uint8_t level = Config::kBrightnessLevels[_brightnessIndex];
    _display.setBrightness(level);
    _display.flush(); // FastLED applies the scale on the next show()
    _settings.setBrightnessDeferred(level);
    LOG_INFO("Brightness %u", level);
}

void App::showInfo()
{
    char buf[112];
    if (_wifi.isConnected())
    {
        snprintf(buf, sizeof(buf), Strings::kInfoOnlineFmt, _wifi.ip().toString().c_str(),
                 _wifi.hostname(), WUA_FIRMWARE_VERSION);
        _alertState.push(buf, Theme::kInfo, 1);
    }
    else if (!_wifi.portalActive())
    {
        _alertState.push(Strings::kInfoOffline, Theme::kInfo, 1);
    }
}

void App::onSettingsSaved()
{
    _alertState.push(Strings::kSettingsSaved, Theme::kInfo, 1);
    _restartAtMs = millis() + Config::kRestartDelayMs;
}

void App::handleSystemEvents()
{
    char buf[64];

    if (_sensors.takeDetectedEvent())
    {
        snprintf(buf, sizeof(buf), Strings::kSensorFoundFmt, _sensors.sensorName());
        _alertState.push(buf, Theme::kInfo, 1);
    }
    if (_sensors.takeLostEvent())
        _alertState.push(Strings::kSensorLost);

    // Report each rejected ticker symbol once.
    const uint32_t now = millis();
    if (now - _lastTickerCheckMs >= 2000)
    {
        _lastTickerCheckMs = now;
        TickerSnapshot snapshot;
        _ticker.read(snapshot);
        for (uint8_t i = 0; i < snapshot.count; ++i)
        {
            const uint8_t bit = static_cast<uint8_t>(1u << i);
            if (snapshot.quotes[i].status == QuoteStatus::Invalid && !(_reportedInvalidTickers & bit))
            {
                _reportedInvalidTickers |= bit;
                snprintf(buf, sizeof(buf), Strings::kTickerInvalidFmt, snapshot.quotes[i].symbol);
                _alertState.push(buf);
            }
        }
    }
}
