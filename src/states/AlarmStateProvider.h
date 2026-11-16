#pragma once

#include "core/Settings.h"
#include "net/TimeService.h"
#include "states/StaticStateBase.h"

// =============================================================================
//  Daily alarm (time set from the portal, "HH:MM").
//
//  The WuaStation-mini has no buzzer or audio amplifier, so the alarm of the
//  WuaDisplay-r1 firmware (MP3 siren over I2S) is ported as a visual alarm:
//  the alarm time flashes in inverted colors for Config::kAlarmRingMs.
//  Any button stops it. Highest priority interrupt screen.
// =============================================================================
class AlarmStateProvider : public StaticStateBase
{
public:
    static constexpr uint8_t kPriority = 4;

    AlarmStateProvider(Compositor &compositor, const TimeService &time, const SettingsStore &settings);

    void init(WuaDisplay &display) override;
    bool canBeShown() const override { return _ringing; }
    bool isFinished() const override { return !_ringing; }
    void update() override;
    bool onButton(ButtonId id, ButtonEvent event) override;

    // Start ringing immediately.
    void ring();

protected:
    void tick() override;
    void render(Adafruit_GFX &canvas) override;

private:
    const TimeService &_time;
    const SettingsStore &_settings;

    bool _enabled = false;
    uint8_t _hour = 0;
    uint8_t _minute = 0;
    int _lastTriggerYearDay = -1;

    bool _ringing = false;
    uint32_t _ringStartMs = 0;
    bool _blinkOn = false;
};
