#include "input/Button.h"

#include "config/Config.h"

void Button::begin()
{
    pinMode(_pin, _inputMode);
    _lastRaw = isPressedRaw();
    _stable = _lastRaw;
    _ignoreUntilRelease = _lastRaw; // a button held at boot never fires
    _lastChangeMs = millis();
}

bool Button::isPressedRaw() const
{
    return digitalRead(_pin) == (_activeHigh ? HIGH : LOW);
}

ButtonEvent Button::poll()
{
    const uint32_t now = millis();
    const bool raw = isPressedRaw();

    if (raw != _lastRaw)
    {
        _lastRaw = raw;
        _lastChangeMs = now;
    }

    if (raw != _stable && now - _lastChangeMs >= Config::kDebounceMs)
    {
        _stable = raw;
        if (_stable)
        {
            _pressedAtMs = now;
            _longFired = false;
        }
        else
        {
            const bool ignored = _ignoreUntilRelease;
            _ignoreUntilRelease = false;
            if (!ignored && !_longFired)
                return ButtonEvent::Click;
        }
    }

    if (_stable && !_longFired && !_ignoreUntilRelease &&
        now - _pressedAtMs >= Config::kLongPressMs)
    {
        _longFired = true;
        return ButtonEvent::LongPress;
    }

    return ButtonEvent::None;
}
