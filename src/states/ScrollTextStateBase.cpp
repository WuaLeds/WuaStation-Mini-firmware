#include "states/ScrollTextStateBase.h"

#include "display/TextUtil.h"

void ScrollTextStateBase::onEnter(WuaDisplay &display)
{
    // The scroll draws straight on the panel: the compositor's last frame is
    // no longer what is on screen.
    _compositor.invalidate();
    _passesDone = 0;
    _finished = false;
    if (!startPass(display))
        finishNow();
}

void ScrollTextStateBase::onLoop(WuaDisplay &display)
{
    if (_finished || display.isScrolling())
        return;

    // One pass is over.
    if (_passesDone < 255)
        ++_passesDone;

    const uint8_t total = passCount();
    if (_duration == 0 && total > 0 && _passesDone >= total)
    {
        finishNow();
        return;
    }
    if (!startPass(display))
        finishNow();
}

bool ScrollTextStateBase::startPass(WuaDisplay &display)
{
    String text;
    Rgb color = Theme::kTickerFlat;
    if (!nextPass(text, color) || text.length() == 0)
        return false;

    display.setTextColorRGB(color.r, color.g, color.b);
    display.startScroll(TextUtil::utf8ToCp437(text.c_str()), _speedMs,
                        WuaDisplay::ScrollMode::Once, WuaDisplay::ScrollDirection::Left,
                        Layout::kTextY);
    return true;
}

void ScrollTextStateBase::finishNow()
{
    if (_finished)
        return;
    _finished = true;
    onFinished();
}
