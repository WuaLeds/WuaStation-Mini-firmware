#pragma once

#include "display/Display.h"

// =============================================================================
//  Compositor -- double-buffered rendering for static screens.
//
//  Static screens draw a full frame into an off-screen canvas (a WuaDisplay
//  Frame, i.e. an Adafruit_GFX GFXcanvas16) and present it either as a hard
//  cut or as a crossfade from the previously presented frame, using the
//  library's non-blocking crossfade engine.
//
//  Scrolling screens draw straight on the panel; they call invalidate() so the
//  next static screen fades in from black instead of from a stale frame.
// =============================================================================
class Compositor
{
public:
    enum class Transition : uint8_t
    {
        Cut,
        Fade
    };

    explicit Compositor(WuaDisplay &display) : _display(display) {}

    // Allocate the frames. Call after display.begin().
    void begin();

    // Cleared back buffer to draw the next frame into.
    WuaDisplay::Frame &beginFrame();

    // Show the back buffer and make it the current (front) frame.
    void present(Transition transition);

    // The panel was drawn by someone else: forget the front frame.
    void invalidate() { _frontValid = false; }

    // A crossfade is running: do not touch the frames until it ends.
    bool busy() const { return _display.isCrossfading(); }

private:
    void blit(const WuaDisplay::Frame &frame);

    WuaDisplay &_display;
    WuaDisplay::Frame *_front = nullptr;
    WuaDisplay::Frame *_back = nullptr;
    bool _frontValid = false;
};
