#include "states/AlarmStateProvider.h"

#include "config/Config.h"
#include "config/Theme.h"
#include "core/Logger.h"
#include "display/TinyFont.h"

AlarmStateProvider::AlarmStateProvider(Compositor &compositor, const TimeService &time, const SettingsStore &settings)
    : StaticStateBase(compositor, "alarm", 0, kPriority), _time(time), _settings(settings)
{
}

void AlarmStateProvider::init(WuaDisplay &display)
{
    (void)display;
    _enabled = _settings.alarmTime(_hour, _minute);
    if (_enabled)
        LOG_INFO("Alarm set at %02u:%02u", static_cast<unsigned>(_hour), static_cast<unsigned>(_minute));
}

void AlarmStateProvider::ring()
{
    _ringing = true;
    _ringStartMs = millis();
    LOG_INFO("Alarm ringing");
}

void AlarmStateProvider::update()
{
    if (_ringing && millis() - _ringStartMs >= Config::kAlarmRingMs)
    {
        _ringing = false;
        LOG_INFO("Alarm timed out");
    }

    if (!_enabled || _ringing)
        return;

    struct tm now;
    if (!_time.localTime(now))
        return;
    if (now.tm_hour == _hour && now.tm_min == _minute && now.tm_yday != _lastTriggerYearDay)
    {
        _lastTriggerYearDay = now.tm_yday; // once per day
        ring();
    }
}

bool AlarmStateProvider::onButton(ButtonId id, ButtonEvent event)
{
    (void)id;
    (void)event;
    if (!_ringing)
        return false;
    _ringing = false; // any button stops the alarm
    LOG_INFO("Alarm dismissed");
    return true;
}

void AlarmStateProvider::tick()
{
    const bool on = ((millis() - _ringStartMs) / Config::kAlarmBlinkMs) % 2 == 0;
    if (on != _blinkOn)
    {
        _blinkOn = on;
        invalidate();
    }
}

void AlarmStateProvider::render(Adafruit_GFX &canvas)
{
    char text[8];
    snprintf(text, sizeof(text), "%02u:%02u", static_cast<unsigned>(_hour), static_cast<unsigned>(_minute));
    const uint16_t accent = Theme::kAlarm.to565();
    if (_blinkOn)
    {
        canvas.fillScreen(accent);
        TinyFont::drawCentered(canvas, text, 0);
    }
    else
    {
        TinyFont::drawCentered(canvas, text, accent);
    }
}
