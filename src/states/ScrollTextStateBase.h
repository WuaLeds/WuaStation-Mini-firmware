#pragma once

#include <Arduino.h>
#include "config/Config.h"
#include "config/Theme.h"
#include "display/Compositor.h"
#include "states/IDisplayStateProvider.h"

// =============================================================================
//  ScrollTextStateBase -- a screen that scrolls one or more messages with the
//  library's non-blocking scroll engine.
//
//  Each "pass" scrolls one message right-to-left until it has fully left the
//  panel; nextPass() provides the text and color of every pass, so a state can
//  rotate through several messages (e.g. one per ticker symbol).
//
//    durationMs > 0  -> keeps scrolling until the time is up
//    durationMs == 0 -> finishes after passCount() passes (0 = never)
// =============================================================================
class ScrollTextStateBase : public IDisplayStateProvider
{
public:
    ScrollTextStateBase(Compositor &compositor, const char *id, uint32_t durationMs = 0, uint8_t passes = 1,
                        uint8_t priority = 0, uint16_t speedMs = Config::kScrollSpeedMs)
        : _compositor(compositor), _id(id), _duration(durationMs), _passes(passes), _priority(priority),
          _speedMs(speedMs)
    {
    }

    const char *getId() const override { return _id; }
    uint32_t getDurationMs() const override { return _duration; }
    // A duration turns "N passes" into "keep scrolling for this long".
    void setDurationMs(uint32_t ms) override { _duration = ms; }
    uint8_t priority() const override { return _priority; }
    bool isFinished() const override { return _finished; }

    void onEnter(WuaDisplay &display) override;
    void onLoop(WuaDisplay &display) override;

protected:
    // Text (UTF-8) and color for the next pass. Return false if there is
    // nothing to show: the state finishes.
    virtual bool nextPass(String &text, Rgb &color) = 0;

    virtual uint8_t passCount() const { return _passes; }

    // Called once when the state finishes by itself (or via finishNow()).
    virtual void onFinished() {}

    // End the state immediately (e.g. dismissed with a button).
    void finishNow();

    uint8_t passesDone() const { return _passesDone; }

private:
    bool startPass(WuaDisplay &display);

    Compositor &_compositor;
    const char *_id;
    uint32_t _duration;
    uint8_t _passes;
    uint8_t _priority;
    uint16_t _speedMs;

    uint8_t _passesDone = 0;
    bool _finished = false;
};
