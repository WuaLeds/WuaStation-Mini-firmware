#pragma once

#include <Arduino.h>
#include "display/Display.h"
#include "input/InputEvents.h"

// =============================================================================
//  IDisplayStateProvider -- one "screen" of the station (time, weather...).
//
//  Same contract as in WuaDisplay-r1, plus two additions:
//    * priority(): 0 = regular screen in the rotation; > 0 = interrupt screen
//      (alert, portal, alarm) that preempts any lower priority as soon as
//      canBeShown() becomes true, and hands back to the rotation when done.
//    * onButton(): lets the active screen consume a button (e.g. dismiss).
// =============================================================================
class IDisplayStateProvider
{
public:
    virtual ~IDisplayStateProvider() = default;

    // Called once when the state is added to the DisplayController.
    virtual void init(WuaDisplay &display) { (void)display; }

    virtual const char *getId() const = 0;

    // 0 = rotation; higher values preempt lower ones.
    virtual uint8_t priority() const { return 0; }

    // How long the state stays on screen (ms). 0 = until isFinished().
    virtual uint32_t getDurationMs() const = 0;

    // Whether the state has something to show right now.
    virtual bool canBeShown() const { return true; }

    virtual void onEnter(WuaDisplay &display) = 0;
    virtual void onLoop(WuaDisplay &display) = 0;
    virtual void onExit(WuaDisplay &display) { (void)display; }

    // Called on every controller loop, active or not (background work).
    virtual void update() {}

    // For duration-less states: leave as soon as this returns true.
    virtual bool isFinished() const { return false; }

    // Return true if the event was consumed.
    virtual bool onButton(ButtonId id, ButtonEvent event)
    {
        (void)id;
        (void)event;
        return false;
    }
};
