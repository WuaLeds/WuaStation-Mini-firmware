#include "display/Compositor.h"

#include "config/Config.h"

void Compositor::begin()
{
    if (_front)
        return;

    // Frames are owned for the whole lifetime of the firmware (never deleted).
    _front = _display.createFrame();
    _back = _display.createFrame();
    _front->cp437(true);
    _back->cp437(true);
    _front->fillScreen(0);
    _back->fillScreen(0);
}

WuaDisplay::Frame &Compositor::beginFrame()
{
    _back->fillScreen(0);
    return *_back;
}

void Compositor::present(Transition transition)
{
    if (transition == Transition::Fade)
    {
        if (!_frontValid)
            _front->fillScreen(0); // fade in from black
        _display.startCrossfade(*_front, *_back, Config::kFadeMs, Config::kFadeStepMs);
    }
    else
    {
        blit(*_back);
    }

    // The presented frame becomes the front one. During a crossfade both
    // frames stay referenced by the engine, which is why callers must wait for
    // busy() == false before drawing the next frame.
    WuaDisplay::Frame *shown = _back;
    _back = _front;
    _front = shown;
    _frontValid = true;
}

void Compositor::blit(const WuaDisplay::Frame &frame)
{
    // GFXcanvas16::getBuffer() is non-const in Adafruit GFX.
    const uint16_t *pixels = const_cast<WuaDisplay::Frame &>(frame).getBuffer();
    auto &panel = _display.panel();
    for (int16_t y = 0; y < Layout::kHeight; ++y)
    {
        for (int16_t x = 0; x < Layout::kWidth; ++x)
        {
            panel.drawPixel(x, y, pixels[y * Layout::kWidth + x]);
        }
    }
    _display.flush();
}
