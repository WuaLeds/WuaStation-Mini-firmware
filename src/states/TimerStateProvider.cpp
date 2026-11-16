#include "states/TimerStateProvider.h"

#include "core/Logger.h"
#include "display/Icons.h"
#include "display/TinyFont.h"
#include "display/Widgets.h"

namespace
{
    constexpr uint32_t kMinuteMs = 60UL * 1000UL;
    constexpr uint32_t kPausedBlinkMs = 500;
}

TimerStateProvider::TimerStateProvider(Compositor &compositor)
    : StaticStateBase(compositor, "timer", Config::kTimerScreenMs)
{
}

// -----------------------------------------------------------------------------
//  Controller hooks
// -----------------------------------------------------------------------------
uint8_t TimerStateProvider::priority() const
{
    const bool active = _run == Run::Running || _run == Run::Paused;
    return (_run == Run::Ringing || (_pinned && active)) ? kPriority : 0;
}

uint32_t TimerStateProvider::getDurationMs() const
{
    if (priority() > 0)
        return 0; // pinned: stays until unpinned / dismissed
    if (_leaveNow)
        return 1; // already over: move on
    const uint32_t base = StaticStateBase::getDurationMs();
    if (!_interacted)
        return base;
    // Stay while the user is setting it up.
    const uint32_t sinceEnter = shownMs();
    const uint32_t sinceButton = millis() - _lastInteractionMs;
    const uint32_t untilHoldEnds = (sinceEnter - sinceButton) + Config::kTimerHoldMs;
    return untilHoldEnds > base ? untilHoldEnds : base;
}

void TimerStateProvider::update()
{
    if (_run == Run::Running && static_cast<int32_t>(millis() - _endMs) >= 0)
        ring();

    if (_run == Run::Ringing)
    {
        const uint32_t ringMs = _phase == Phase::Countdown ? Config::kTimerRingMs : Config::kPomodoroRingMs;
        if (millis() - _ringStartMs >= ringMs)
            dismiss();
    }
}

bool TimerStateProvider::onButton(ButtonId id, ButtonEvent event)
{
    if (_run == Run::Ringing)
    {
        dismiss(); // any button acknowledges
        return true;
    }

    if (id == ButtonId::Down)
    {
        noteInteraction();
        switch (_run)
        {
        case Run::Idle:
            if (event == ButtonEvent::Click)
                _preset = static_cast<uint8_t>((_preset + 1) % (kPresetCount + 1));
            else if (event == ButtonEvent::LongPress)
            {
                _workBlocks = 0;
                _pinned = true;
                start(pomodoroSelected() ? Phase::Work : Phase::Countdown);
            }
            break;
        case Run::Running:
            event == ButtonEvent::Click ? pause() : stop();
            break;
        case Run::Paused:
            event == ButtonEvent::Click ? resume() : stop();
            break;
        case Run::Ringing:
            break;
        }
        invalidate();
        return true;
    }

    // UP / TOUCH click on a pinned timer: let the rotation go on.
    if (event == ButtonEvent::Click && priority() > 0)
    {
        _pinned = false;
        _leaveNow = true;
        return true;
    }
    return false; // global action (next screen, portal, info)
}

// -----------------------------------------------------------------------------
//  Timer logic
// -----------------------------------------------------------------------------
uint32_t TimerStateProvider::phaseMs(Phase phase) const
{
    switch (phase)
    {
    case Phase::Work:
        return Config::kPomodoroWorkMin * kMinuteMs;
    case Phase::ShortBreak:
        return Config::kPomodoroShortBreakMin * kMinuteMs;
    case Phase::LongBreak:
        return Config::kPomodoroLongBreakMin * kMinuteMs;
    case Phase::Countdown:
    default:
        return Config::kTimerPresetsMin[_preset < kPresetCount ? _preset : 0] * kMinuteMs;
    }
}

void TimerStateProvider::start(Phase phase)
{
    _phase = phase;
    _run = Run::Running;
    _endMs = millis() + phaseMs(phase);
    LOG_INFO("Timer: phase %u, %lu s", static_cast<unsigned>(phase),
             static_cast<unsigned long>(phaseMs(phase) / 1000));
}

void TimerStateProvider::pause()
{
    _remainingMs = remainingMs();
    _run = Run::Paused;
}

void TimerStateProvider::resume()
{
    _endMs = millis() + _remainingMs;
    _run = Run::Running;
}

void TimerStateProvider::stop()
{
    _run = Run::Idle;
    _pinned = false;
    _workBlocks = 0;
    LOG_INFO("Timer stopped");
}

void TimerStateProvider::ring()
{
    _run = Run::Ringing;
    _ringStartMs = millis();
    if (_phase == Phase::Work)
        ++_workBlocks;
    LOG_INFO("Timer finished");
}

void TimerStateProvider::dismiss()
{
    if (_phase == Phase::Countdown)
    {
        _run = Run::Idle;
        _pinned = false;
        _leaveNow = true;
        return;
    }

    // Pomodoro: work -> short break (long break every N blocks) -> work ...
    Phase next = Phase::Work;
    if (_phase == Phase::Work)
        next = (_workBlocks % Config::kPomodoroCycles == 0) ? Phase::LongBreak : Phase::ShortBreak;
    start(next);
    if (!_pinned)
        _leaveNow = true;
}

void TimerStateProvider::noteInteraction()
{
    _interacted = true;
    _lastInteractionMs = millis();
}

uint32_t TimerStateProvider::remainingMs() const
{
    if (_run == Run::Paused)
        return _remainingMs;
    if (_run != Run::Running)
        return 0;
    const int32_t left = static_cast<int32_t>(_endMs - millis());
    return left > 0 ? static_cast<uint32_t>(left) : 0;
}

Rgb TimerStateProvider::color() const
{
    if (_run == Run::Idle)
        return pomodoroSelected() ? Theme::kPomodoroWork : Theme::kTimerCountdown;
    switch (_phase)
    {
    case Phase::Work:
        return Theme::kPomodoroWork;
    case Phase::ShortBreak:
    case Phase::LongBreak:
        return Theme::kPomodoroBreak;
    case Phase::Countdown:
    default:
        return Theme::kTimerCountdown;
    }
}

// -----------------------------------------------------------------------------
//  Rendering
// -----------------------------------------------------------------------------
void TimerStateProvider::onShow()
{
    _leaveNow = false;
    _interacted = false;
    _shownKey = renderKey() + 1; // force the first tick to compare as changed
}

uint32_t TimerStateProvider::renderKey() const
{
    // Everything that changes the picture, packed in one number.
    const uint32_t now = millis();
    uint32_t key = static_cast<uint32_t>(_run) | (static_cast<uint32_t>(_phase) << 2) |
                   (static_cast<uint32_t>(_preset) << 4);
    switch (_run)
    {
    case Run::Running:
        key |= ((remainingMs() + 999) / 1000) << 9;
        break;
    case Run::Paused:
        key |= ((now / kPausedBlinkMs) & 1) << 9;
        break;
    case Run::Ringing:
        key |= (((now - _ringStartMs) / Config::kAlarmBlinkMs) & 1) << 9;
        break;
    case Run::Idle:
        break;
    }
    return key;
}

void TimerStateProvider::tick()
{
    const uint32_t key = renderKey();
    if (key != _shownKey)
        invalidate();
}

void TimerStateProvider::render(Adafruit_GFX &canvas)
{
    _shownKey = renderKey();
    const uint16_t c = color().to565();
    char text[8];

    if (_run == Run::Idle)
    {
        const unsigned minutes = pomodoroSelected() ? Config::kPomodoroWorkMin : Config::kTimerPresetsMin[_preset];
        snprintf(text, sizeof(text), "%u", minutes);
        Widgets::iconAndValue(canvas, pomodoroSelected() ? Icons::kTomato : Icons::kHourglass, text, c);
        return;
    }

    if (_run == Run::Ringing)
    {
        const bool on = (((millis() - _ringStartMs) / Config::kAlarmBlinkMs) & 1) == 0;
        if (on)
        {
            canvas.fillScreen(c);
            TinyFont::drawCentered(canvas, "00:00", 0);
        }
        else
        {
            TinyFont::drawCentered(canvas, "00:00", c);
        }
        return;
    }

    // Running / paused: MM:SS (presets top out at 90 min, so 2 digits are enough).
    if (_run == Run::Paused && ((millis() / kPausedBlinkMs) & 1))
        return; // blink while paused

    const uint32_t seconds = (remainingMs() + 999) / 1000;
    snprintf(text, sizeof(text), "%02u:%02u", static_cast<unsigned>(seconds / 60),
             static_cast<unsigned>(seconds % 60));
    TinyFont::drawCentered(canvas, text, c);
}
