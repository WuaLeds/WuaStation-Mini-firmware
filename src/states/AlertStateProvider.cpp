#include "states/AlertStateProvider.h"

#include "core/Logger.h"

void AlertStateProvider::push(const char *message, Rgb color, uint8_t loops)
{
    if (_count == kQueueSize)
    {
        _head = (_head + 1) % kQueueSize; // drop the oldest
        --_count;
    }
    Entry &e = _queue[(_head + _count) % kQueueSize];
    strlcpy(e.text, message, sizeof(e.text));
    e.color = color;
    e.loops = loops == 0 ? 1 : loops;
    ++_count;
    LOG_INFO("Alert: %s", message);
}

void AlertStateProvider::onEnter(WuaDisplay &display)
{
    if (_count > 0)
    {
        _current = _queue[_head];
        _head = (_head + 1) % kQueueSize;
        --_count;
        _active = true;
    }
    ScrollTextStateBase::onEnter(display);
}

bool AlertStateProvider::nextPass(String &text, Rgb &color)
{
    if (!_active)
        return false;
    text = _current.text;
    color = _current.color;
    return true;
}

bool AlertStateProvider::onButton(ButtonId id, ButtonEvent event)
{
    (void)id;
    if (!_active || event != ButtonEvent::Click)
        return false;
    finishNow(); // dismiss the current message
    return true;
}
