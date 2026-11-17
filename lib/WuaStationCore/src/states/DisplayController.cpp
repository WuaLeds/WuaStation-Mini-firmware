#include "states/DisplayController.h"

#include "core/Logger.h"

bool DisplayController::addState(IDisplayStateProvider *state)
{
    if (!state || _count >= kMaxStates)
        return false;
    state->init(_display);
    _states[_count++] = state;
    return true;
}

void DisplayController::begin()
{
    _started = true;
    _current = -1;
    _resume = -1;
}

bool DisplayController::isDone(const IDisplayStateProvider *state) const
{
    if (!state->canBeShown())
        return true;
    const uint32_t duration = state->getDurationMs();
    if (duration > 0)
        return millis() - _enteredMs >= duration;
    return state->isFinished();
}

int8_t DisplayController::findRotation(int8_t from, int8_t step, bool includeFrom) const
{
    if (_count == 0)
        return -1;
    int16_t idx = from < 0 ? 0 : from;
    if (from < 0)
        includeFrom = true;
    // Visits every state once. Without includeFrom, `from` itself is checked
    // last (after wrapping around), so a lone showable state is re-entered.
    for (uint8_t i = 0; i < _count; ++i)
    {
        if (!(includeFrom && i == 0))
            idx = (idx + step + _count) % _count;
        const IDisplayStateProvider *s = _states[idx];
        if (s->priority() == 0 && s->canBeShown())
            return static_cast<int8_t>(idx);
    }
    return -1;
}

int8_t DisplayController::findInterrupt() const
{
    int8_t best = -1;
    uint8_t bestPriority = 0;
    for (uint8_t i = 0; i < _count; ++i)
    {
        const IDisplayStateProvider *s = _states[i];
        if (s->priority() > bestPriority && s->canBeShown())
        {
            best = static_cast<int8_t>(i);
            bestPriority = s->priority();
        }
    }
    return best;
}

void DisplayController::switchTo(int8_t index)
{
    if (_current >= 0)
        _states[_current]->onExit(_display);

    // Never let the previous screen keep animating over the new one.
    _display.stopScroll();
    _display.stopCrossfade();

    _current = index;
    _enteredMs = millis();
    LOG_DEBUG("Screen -> %s", _states[index]->getId());
    _states[index]->onEnter(_display);
}

void DisplayController::showNothing()
{
    if (_current >= 0)
        _states[_current]->onExit(_display);
    _display.stopScroll();
    _display.stopCrossfade();
    _current = -1;
    _display.clear();
    _display.flush();
    _compositor.invalidate();
}

void DisplayController::loop()
{
    // 1) Background work for every state (data refresh, alarm checks...).
    for (uint8_t i = 0; i < _count; ++i)
        _states[i]->update();

    // 2) Drive the library animations (scroll / crossfade).
    _display.update();

    if (!_started || _count == 0)
        return;

    // 3) Interrupt screens preempt anything with a lower priority.
    const uint8_t currentPriority = _current >= 0 ? _states[_current]->priority() : 0;
    const int8_t irq = findInterrupt();
    if (irq >= 0 && irq != _current && _states[irq]->priority() > currentPriority)
    {
        if (_current >= 0 && currentPriority == 0)
            _resume = _current;
        switchTo(irq);
        return;
    }

    // 4) Nothing on screen yet: pick the first showable rotation state.
    if (_current < 0)
    {
        const int8_t idx = findRotation(_resume, +1, true);
        if (idx >= 0)
        {
            _resume = -1;
            switchTo(idx);
        }
        return;
    }

    IDisplayStateProvider *state = _states[_current];
    state->onLoop(_display);
    if (!isDone(state))
        return;

    // 5) The active state is over: pick what comes next.
    if (state->priority() > 0)
    {
        const int8_t nextIrq = findInterrupt();
        if (nextIrq >= 0)
        {
            switchTo(nextIrq); // e.g. the next queued alert
            return;
        }
        const int8_t idx = findRotation(_resume, +1, true);
        _resume = -1;
        if (idx >= 0)
            switchTo(idx);
        else
            showNothing();
        return;
    }

    const int8_t idx = findRotation(_current, +1, false);
    if (idx >= 0)
        switchTo(idx);
    else
        showNothing();
}

void DisplayController::next()
{
    if (_current >= 0 && _states[_current]->priority() > 0)
        return;
    const int8_t idx = findRotation(_current, +1, false);
    if (idx >= 0)
        switchTo(idx);
}

void DisplayController::previous()
{
    if (_current >= 0 && _states[_current]->priority() > 0)
        return;
    const int8_t idx = findRotation(_current, -1, false);
    if (idx >= 0)
        switchTo(idx);
}

int8_t DisplayController::indexOf(const IDisplayStateProvider *state) const
{
    for (uint8_t i = 0; i < _count; ++i)
    {
        if (_states[i] == state)
            return static_cast<int8_t>(i);
    }
    return -1;
}

bool DisplayController::show(const IDisplayStateProvider *state)
{
    const int8_t idx = indexOf(state);
    if (idx < 0 || _states[idx]->priority() > 0 || !_states[idx]->canBeShown())
        return false;
    if (_current >= 0 && _states[_current]->priority() > 0)
    {
        _resume = idx; // right after the alert / alarm / portal
        return true;
    }
    if (idx != _current)
        switchTo(idx);
    return true;
}

void DisplayController::beginRebuild()
{
    _rebuildCurrent = _current >= 0 ? _states[_current] : nullptr;
    _rebuildResume = _resume >= 0 ? _states[_resume] : nullptr;
    _count = 0;
    _current = -1;
    _resume = -1;
}

void DisplayController::endRebuild()
{
    const int8_t current = indexOf(_rebuildCurrent);
    if (current >= 0)
    {
        _current = current; // still registered: keep it on display untouched
    }
    else if (_rebuildCurrent)
    {
        // Its screen was turned off: the next loop picks what comes next.
        _rebuildCurrent->onExit(_display);
        _display.stopScroll();
        _display.stopCrossfade();
        _compositor.invalidate();
    }
    _resume = indexOf(_rebuildResume);
    _rebuildCurrent = nullptr;
    _rebuildResume = nullptr;
}

bool DisplayController::dispatchButton(ButtonId id, ButtonEvent event)
{
    return _current >= 0 && _states[_current]->onButton(id, event);
}
