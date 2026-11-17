#pragma once

#include <Arduino.h>
#include "display/Compositor.h"
#include "states/IDisplayStateProvider.h"

// =============================================================================
//  StaticStateBase -- a screen made of full frames (clock, weather...).
//
//  Derived classes only implement render(): draw the whole frame onto the
//  given canvas. The base class crossfades into the screen on entry and
//  redraws (hard cut) whenever invalidate() is called, typically from tick()
//  when the content changes (new minute, new data, animation step).
// =============================================================================
class StaticStateBase : public IDisplayStateProvider
{
public:
    StaticStateBase(Compositor &compositor, const char *id, uint32_t durationMs, uint8_t priority = 0)
        : _compositor(compositor), _id(id), _duration(durationMs), _priority(priority)
    {
    }

    const char *getId() const override { return _id; }
    uint32_t getDurationMs() const override { return _duration; }
    void setDurationMs(uint32_t ms) override { _duration = ms; }
    uint8_t priority() const override { return _priority; }

    void onEnter(WuaDisplay &display) override;
    void onLoop(WuaDisplay &display) override;

protected:
    // Reset per-visit state (called before the first render of each visit).
    virtual void onShow() {}

    // Called every loop while active: detect changes and call invalidate().
    virtual void tick() {}

    // Draw the complete frame. The canvas is already cleared.
    virtual void render(Adafruit_GFX &canvas) = 0;

    // Request a redraw. Cut for live updates (clock, animations), Fade when
    // the screen switches to different content (e.g. ticker name -> price).
    void invalidate(Compositor::Transition transition = Compositor::Transition::Cut)
    {
        _dirty = true;
        if (transition == Compositor::Transition::Fade)
            _dirtyFade = true;
    }

    // Time since the state was entered.
    uint32_t shownMs() const { return millis() - _enteredMs; }

private:
    void draw(Compositor::Transition transition);

    Compositor &_compositor;
    const char *_id;
    uint32_t _duration;
    uint8_t _priority;
    uint32_t _enteredMs = 0;
    bool _dirty = false;
    bool _dirtyFade = false;
};
