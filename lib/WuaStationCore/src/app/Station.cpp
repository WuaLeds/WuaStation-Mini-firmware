#include "app/Station.h"

#include "board/Board.h"
#include "board/Pins.h"
#include "config/Config.h"
#include "config/ScreenConfig.h"
#include "config/Strings.h"
#include "config/Theme.h"
#include "core/Logger.h"

namespace
{
    constexpr uint8_t kBrightnessLevelCount = sizeof(Config::kBrightnessLevels) / sizeof(Config::kBrightnessLevels[0]);

    const CalendarStateProvider::Date kCelebrationDate{Config::kCelebrationYear, Config::kCelebrationMonth,
                                                       Config::kCelebrationDay};
}

Station::Station(Host &host)
    : _host(host), _compositor(_display), _controller(_display, _compositor),
      _btnUp(Pins::kButtonUp, false, INPUT_PULLUP), _btnDown(Pins::kButtonDown, false, INPUT_PULLUP),
      _btnTouch(Pins::kTouch, true, INPUT), _splashState(_compositor), _statusState(_compositor, host, _time),
      _timeState(_compositor, _time), _dateState(_compositor, _time), _weatherState(_compositor, _weather),
      _forecastState(_compositor, _weather), _indoorState(_compositor, _sensors), _tickerState(_compositor, _ticker),
      _messageState(_compositor, _settings), _celebrationState(_compositor, _time, kCelebrationDate),
      _timerState(_compositor), _portalState(_compositor, host), _alertState(_compositor),
      _alarmState(_compositor, _time, _settings)
{
}

// -----------------------------------------------------------------------------
//  Setup
// -----------------------------------------------------------------------------
void Station::setup()
{
    Board::init();
    LOG_INFO("WuaStation-mini v%s (id %s)", WUA_FIRMWARE_VERSION, Board::deviceId());

    _settings.begin();
    const Settings &s = _settings.get();

    // Display (WuaDisplay-LMX, LMX1p backend)
    _display.begin();
    _display.panel().cp437(true); // real CP437 glyphs (accents, degree sign)
    _display.setBrightness(s.brightness);
    _compositor.begin();
    syncBrightnessStep(s.brightness);

    _btnUp.begin();
    _btnDown.begin();
    _btnTouch.begin();
    checkFactoryReset();

    // Clock and data sources (city and tickers are configured once: changing
    // them restarts the station)
    _time.begin(s.timezone, _settings.locationZone());
    _ticker.configure(s.tickers);
    float lat = 0.0f, lon = 0.0f;
    const bool hasCoords = _settings.coordinates(lat, lon);
    _weather.configure(s.city, hasCoords, lat, lon, s.europeanAqi);
    _net.add(&_ticker);
    _net.add(&_weather);
    _sensors.setTemperatureOffset(s.tempOffsetC);
    _sensors.begin();

    registerStates();
    _controller.begin();
    _net.begin();

    if (!_time.timezoneKnown())
    {
        char buf[96];
        snprintf(buf, sizeof(buf), Strings::kTimezoneUnknownFmt, s.timezone);
        _alertState.push(buf);
    }

    LOG_INFO("Setup done, free heap %u", ESP.getFreeHeap());
}

IDisplayStateProvider *Station::screen(ScreenId id)
{
    switch (id)
    {
    case ScreenId::Time: return &_timeState;
    case ScreenId::Date: return &_dateState;
    case ScreenId::Weather: return &_weatherState;
    case ScreenId::Forecast: return &_forecastState;
    case ScreenId::Indoor: return &_indoorState;
    case ScreenId::Ticker: return &_tickerState;
    case ScreenId::Message: return &_messageState;
    case ScreenId::Celebration: return &_celebrationState;
    case ScreenId::Timer: return &_timerState;
    case ScreenId::Count: break;
    }
    return nullptr;
}

void Station::registerStates()
{
    // System screens first: boot greeting and connection status.
    _controller.addState(&_splashState);
    _controller.addState(&_statusState);

    // User rotation from the settings, in its order. Screens without data are
    // skipped at runtime; hidden or unlisted screens are never shown.
    ScreenSlot slots[ScreenConfig::kMaxSlots];
    const uint8_t count = ScreenConfig::parse(_settings.get().screens, slots);
    for (uint8_t i = 0; i < count; ++i)
    {
        IDisplayStateProvider *state = screen(slots[i].id);
        if (!state || !slots[i].enabled)
            continue;
        // A screen back to its default time after a custom one: reset it too.
        state->setDurationMs(slots[i].seconds > 0 ? static_cast<uint32_t>(slots[i].seconds) * 1000UL
                                                  : ScreenConfig::defaultMs(slots[i].id));
        _controller.addState(state);
        LOG_DEBUG("Screen %u: %s%s", i + 1, ScreenConfig::name(slots[i].id), slots[i].seconds ? "" : " (default time)");
    }

    // Interrupt screens (priority > 0).
    _controller.addState(&_portalState);
    _controller.addState(&_alertState);
    _controller.addState(&_alarmState);
}

void Station::rebuildRotation()
{
    _controller.beginRebuild();
    registerStates();
    _controller.endRebuild();
}

void Station::checkFactoryReset()
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
            delay(1000);
            factoryReset();
            return;
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
void Station::loop()
{
    // The host runs the WiFi: follow its view of the link.
    const bool online = _host.isConnected();
    _net.setOnline(online);
    if (online)
        _time.startSync(); // once; no-op afterwards

    handleInput();
    _sensors.loop();
    _settings.loop();
    handleSystemEvents();
    _controller.loop();

    if (_controller.current() != _lastShown)
    {
        _lastShown = _controller.current();
        if (_listener)
            _listener->onScreenChanged();
    }

    if (_restartAtMs != 0 && static_cast<int32_t>(millis() - _restartAtMs) >= 0)
    {
        _restartAtMs = 0;
        LOG_INFO("Restarting to apply the new settings");
        _host.restart();
    }
}

void Station::handleInput()
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

void Station::handleButton(ButtonId id, ButtonEvent event)
{
    LOG_DEBUG("Button %u event %u", static_cast<unsigned>(id), static_cast<unsigned>(event));
    if (_listener)
        _listener->onButton(id, event); // Home Assistant sees every press

    // The screen on display gets the first chance (dismiss alarm/alert...).
    if (_controller.dispatchButton(id, event))
        return;

    switch (id)
    {
    case ButtonId::Up:
        if (event == ButtonEvent::Click)
            _controller.next();
        else if (event == ButtonEvent::LongPress)
            showWifiInfo();
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

// -----------------------------------------------------------------------------
//  Actions
// -----------------------------------------------------------------------------
void Station::stepBrightness()
{
    // A dark display comes back at its brightness first.
    if (!_displayOn)
    {
        setDisplayOn(true);
        return;
    }
    _brightnessIndex = (_brightnessIndex + 1) % kBrightnessLevelCount;
    setBrightness(Config::kBrightnessLevels[_brightnessIndex]);
}

void Station::syncBrightnessStep(uint8_t level)
{
    // Keep the button steps in sync: the next click goes to the step above.
    _brightnessIndex = 0;
    for (uint8_t i = 0; i < kBrightnessLevelCount; ++i)
    {
        if (Config::kBrightnessLevels[i] <= level)
            _brightnessIndex = i;
    }
}

void Station::setBrightness(uint8_t level)
{
    level = level == 0 ? 1 : level;
    _displayOn = true;
    _display.setBrightness(level);
    _display.flush(); // FastLED applies the scale on the next show()
    _settings.setBrightnessDeferred(level);
    syncBrightnessStep(level);
    LOG_INFO("Brightness %u", level);
    settingsChanged();
}

void Station::setDisplayOn(bool on)
{
    if (on == _displayOn)
        return;
    _displayOn = on;
    _display.setBrightness(on ? _settings.get().brightness : 0);
    _display.flush();
    LOG_INFO("Display %s", on ? "on" : "off");
    settingsChanged();
}

void Station::setMessage(const char *text)
{
    Settings s = _settings.get();
    strlcpy(s.message, text ? text : "", sizeof(s.message));
    if (strcmp(s.message, _settings.get().message) == 0)
        return;
    _settings.update(s);
    settingsChanged();
}

void Station::setScreenEnabled(ScreenId id, bool enabled)
{
    Settings s = _settings.get();
    ScreenConfig::setEnabled(s.screens, sizeof(s.screens), id, enabled);
    if (strcmp(s.screens, _settings.get().screens) == 0)
        return;
    _settings.update(s);
    rebuildRotation();
    settingsChanged();
}

bool Station::currentScreen(ScreenId &out) const
{
    const IDisplayStateProvider *current = _controller.current();
    if (!current)
        return false;
    for (uint8_t i = 0; i < ScreenConfig::kMaxSlots; ++i)
    {
        const ScreenId id = static_cast<ScreenId>(i);
        if (const_cast<Station *>(this)->screen(id) == current)
        {
            out = id;
            return true;
        }
    }
    return false;
}

void Station::showScreen(ScreenId id)
{
    if (!_controller.show(screen(id)))
        LOG_INFO("Screen %s is hidden or has nothing to show", ScreenConfig::name(id));
}

void Station::nextScreen()
{
    _controller.next();
}

void Station::notify(const char *text, Rgb color, uint8_t loops)
{
    if (text && text[0])
        _alertState.push(text, color, loops == 0 ? 1 : loops);
}

bool Station::applySettings(const Settings &updated)
{
    const Settings before = _settings.get();
    _settings.update(updated);
    const Settings &after = _settings.get();

    // Live changes
    if (after.brightness != before.brightness && _displayOn)
    {
        _display.setBrightness(after.brightness);
        _display.flush();
        syncBrightnessStep(after.brightness);
    }
    if (after.tempOffsetC != before.tempOffsetC)
        _sensors.setTemperatureOffset(after.tempOffsetC);
    if (after.europeanAqi != before.europeanAqi)
        _weather.setEuropeanAqi(after.europeanAqi);
    if (strcmp(after.timezone, before.timezone) != 0)
    {
        _time.begin(after.timezone, _settings.locationZone());
        if (!_time.timezoneKnown())
        {
            char buf[96];
            snprintf(buf, sizeof(buf), Strings::kTimezoneUnknownFmt, after.timezone);
            _alertState.push(buf);
        }
    }
    if (strcmp(after.screens, before.screens) != 0 || strcmp(after.alarm, before.alarm) != 0)
        rebuildRotation(); // the alarm screen reads its time when added

    settingsChanged();

    // Data sources are configured once at boot.
    const bool restart = SettingsStore::needsRestart(before, after);
    if (restart)
    {
        _alertState.push(Strings::kSettingsSaved, Theme::kInfo, 1);
        restartSoon(Config::kRestartDelayMs);
    }
    return restart;
}

void Station::restartSoon(uint32_t delayMs)
{
    _restartAtMs = millis() + delayMs;
    if (_restartAtMs == 0)
        _restartAtMs = 1; // 0 means "no restart pending"
}

void Station::factoryReset()
{
    LOG_WARN("Factory reset");
    _settings.factoryReset();
    _host.eraseHostData();
    _host.restart();
}

void Station::settingsChanged()
{
    if (_listener)
        _listener->onSettingsChanged();
}

void Station::showInfo()
{
    char buf[112];
    if (_host.isConnected())
    {
        char ip[16] = "";
        _host.ip(ip, sizeof(ip));
        snprintf(buf, sizeof(buf), Strings::kInfoOnlineFmt, ip, _host.hostname(), WUA_FIRMWARE_VERSION,
                 _host.homeAssistantConnected() ? Strings::kInfoHomeAssistant : "");
        _alertState.push(buf, Theme::kInfo, 1);
    }
    else if (!_host.setupActive())
    {
        _alertState.push(Strings::kInfoOffline, Theme::kInfo, 1);
    }
}

void Station::showWifiInfo()
{
    if (_host.setupActive())
        return; // the setup instructions are already scrolling
    if (!_host.isConnected())
    {
        char buf[128];
        if (_host.apActive())
            snprintf(buf, sizeof(buf), Strings::kPortalFmt, _host.apName());
        else
            strlcpy(buf, Strings::kInfoOffline, sizeof(buf));
        _alertState.push(buf, Theme::kPortal, 1);
        return;
    }
    char ip[16] = "";
    char ssid[33] = "";
    char buf[112];
    _host.ip(ip, sizeof(ip));
    _host.ssid(ssid, sizeof(ssid));
    snprintf(buf, sizeof(buf), Strings::kWifiInfoFmt, ssid, ip, _host.hostname());
    _alertState.push(buf, Theme::kInfo, 1);
}

void Station::handleSystemEvents()
{
    char buf[64];

    // Timezone "auto": follow the zone reported for the weather location.
    if (_time.isAuto() && _weather.version() != _weatherZoneVersion)
    {
        WeatherData weather;
        _weatherZoneVersion = _weather.read(weather);
        if (weather.valid && weather.hasTimezone && _time.applyLocationZone(weather.zoneName, weather.utcOffsetS))
            _settings.rememberLocationZone(_time.posixTimezone());
    }

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
