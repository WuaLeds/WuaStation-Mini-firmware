#pragma once

#include <Arduino.h>
#include "display/Compositor.h"
#include "display/Display.h"
#include "states/IDisplayStateProvider.h"

// =============================================================================
//  DisplayController -- the screen state machine.
//
//  * Rotation: priority-0 states are shown in registration order, skipping
//    the ones that cannot be shown (no data yet...).
//  * Interrupts: a state with priority > 0 takes over as soon as it can be
//    shown; when it ends, the rotation resumes where it was.
//  * On every switch the previous scroll/crossfade is stopped, so a screen
//    never draws over another one.
// =============================================================================
class DisplayController
{
public:
    static constexpr uint8_t kMaxStates = 24;

    DisplayController(WuaDisplay &display, Compositor &compositor)
        : _display(display), _compositor(compositor) {}

    bool addState(IDisplayStateProvider *state);
    void begin();
    void loop();

    // Manual navigation through the rotation (ignored during interrupts).
    void next();
    void previous();

    // Offer a button event to the active state. True if it was consumed.
    bool dispatchButton(ButtonId id, ButtonEvent event);

    const IDisplayStateProvider *current() const
    {
        return _current >= 0 ? _states[_current] : nullptr;
    }

private:
    bool isDone(const IDisplayStateProvider *state) const;
    int8_t findRotation(int8_t from, int8_t step, bool includeFrom) const;
    int8_t findInterrupt() const;
    void switchTo(int8_t index);
    void showNothing();

    WuaDisplay &_display;
    Compositor &_compositor;
    IDisplayStateProvider *_states[kMaxStates] = {};
    uint8_t _count = 0;
    int8_t _current = -1;
    int8_t _resume = -1; // rotation state interrupted by an interrupt state
    uint32_t _enteredMs = 0;
    bool _started = false;
};
