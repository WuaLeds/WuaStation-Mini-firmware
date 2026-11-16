#pragma once

#include "config/Config.h"
#include "config/Theme.h"
#include "states/StaticStateBase.h"

// =============================================================================
//  Countdown timer and pomodoro, driven with the DOWN button while the timer
//  screen is on display (UP keeps its "next screen" meaning).
//
//    idle      hourglass / tomato icon + minutes
//              DOWN click: next preset (1..90 min, then pomodoro)
//              DOWN long : start
//    running   "MM:SS"; stays pinned on screen (low-priority interrupt)
//              DOWN click: pause / resume     DOWN long: stop
//              UP / TOUCH click: unpin (keeps running, rotation goes on)
//    finished  flashes "00:00" until any button or the ring time; a pomodoro
//              then starts the next phase on its own (work -> break -> ...)
//
//  Visual only: the board has no buzzer.
// =============================================================================
class TimerStateProvider : public StaticStateBase
{
public:
    static constexpr uint8_t kPriority = 1; // below portal, alerts and the alarm

    explicit TimerStateProvider(Compositor &compositor);

    // Pinned while running and always while flashing; otherwise a regular
    // rotation screen.
    uint8_t priority() const override;
    uint32_t getDurationMs() const override;
    bool isFinished() const override { return priority() == 0; }
    void update() override;
    bool onButton(ButtonId id, ButtonEvent event) override;

protected:
    void onShow() override;
    void tick() override;
    void render(Adafruit_GFX &canvas) override;

private:
    enum class Run : uint8_t
    {
        Idle,
        Running,
        Paused,
        Ringing
    };
    enum class Phase : uint8_t
    {
        Countdown,
        Work,
        ShortBreak,
        LongBreak
    };

    static constexpr uint8_t kPresetCount = sizeof(Config::kTimerPresetsMin);

    bool pomodoroSelected() const { return _preset == kPresetCount; } // last option
    uint32_t phaseMs(Phase phase) const;
    Rgb color() const;
    uint32_t remainingMs() const;
    uint32_t renderKey() const;

    void start(Phase phase);
    void pause();
    void resume();
    void stop();
    void ring();
    void dismiss();
    void noteInteraction();

    uint8_t _preset = Config::kTimerDefaultPreset; // 0..kPresetCount-1 minutes, kPresetCount = pomodoro
    Run _run = Run::Idle;
    Phase _phase = Phase::Countdown;
    bool _pinned = false;
    uint8_t _workBlocks = 0; // completed pomodoro work blocks

    uint32_t _endMs = 0;       // running: when the phase ends
    uint32_t _remainingMs = 0; // paused: what was left
    uint32_t _ringStartMs = 0;

    bool _leaveNow = false;    // ask the controller to move on
    bool _interacted = false;  // a button was pressed during this visit
    uint32_t _lastInteractionMs = 0;
    uint32_t _shownKey = 0;
};
