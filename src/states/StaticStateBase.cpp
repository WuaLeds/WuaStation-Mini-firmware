#include "states/StaticStateBase.h"

void StaticStateBase::onEnter(WuaDisplay &display)
{
    (void)display;
    _enteredMs = millis();
    _dirty = false;
    onShow();
    draw(Compositor::Transition::Fade);
}

void StaticStateBase::onLoop(WuaDisplay &display)
{
    (void)display;
    tick();
    // Wait for the entry crossfade to finish: it still reads both frames.
    if (_dirty && !_compositor.busy())
    {
        _dirty = false;
        draw(Compositor::Transition::Cut);
    }
}

void StaticStateBase::draw(Compositor::Transition transition)
{
    WuaDisplay::Frame &canvas = _compositor.beginFrame();
    render(canvas);
    _compositor.present(transition);
}
