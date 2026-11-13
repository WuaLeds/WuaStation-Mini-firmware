#include "states/StaticStateBase.h"

void StaticStateBase::onEnter(WuaDisplay &display)
{
    (void)display;
    _enteredMs = millis();
    _dirty = false;
    _dirtyFade = false;
    onShow();
    draw(Compositor::Transition::Fade);
}

void StaticStateBase::onLoop(WuaDisplay &display)
{
    (void)display;
    tick();
    // Wait for a running crossfade to finish: it still reads both frames.
    if (_dirty && !_compositor.busy())
    {
        const Compositor::Transition transition =
            _dirtyFade ? Compositor::Transition::Fade : Compositor::Transition::Cut;
        _dirty = false;
        _dirtyFade = false;
        draw(transition);
    }
}

void StaticStateBase::draw(Compositor::Transition transition)
{
    WuaDisplay::Frame &canvas = _compositor.beginFrame();
    render(canvas);
    _compositor.present(transition);
}
