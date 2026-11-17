#pragma once

#include <Arduino.h>
#include "input/InputEvents.h"

// =============================================================================
//  Polled, debounced push button with click / long-press detection.
//  Runs entirely in the main loop: no interrupts, no extra tasks, so event
//  handlers can safely touch the display and the rest of the app.
// =============================================================================
class Button
{
public:
    // activeHigh: logic level when pressed. inputMode: INPUT / INPUT_PULLUP...
    Button(uint8_t pin, bool activeHigh, uint8_t inputMode) : _pin(pin), _activeHigh(activeHigh), _inputMode(inputMode)
    {
    }

    void begin();

    // Call every loop. Returns at most one event per call.
    ButtonEvent poll();

    // Current raw (not debounced) state.
    bool isPressedRaw() const;

private:
    uint8_t _pin;
    bool _activeHigh;
    uint8_t _inputMode;

    bool _stable = false; // debounced state
    bool _lastRaw = false;
    bool _longFired = false;
    bool _ignoreUntilRelease = false; // held at boot
    uint32_t _lastChangeMs = 0;
    uint32_t _pressedAtMs = 0;
};
