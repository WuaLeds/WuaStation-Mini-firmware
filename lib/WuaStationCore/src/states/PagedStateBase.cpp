#include "states/PagedStateBase.h"

void PagedStateBase::onShow()
{
    _page = 0;
    _pageCount = beginPages();
    _finished = _pageCount == 0;
    if (!_finished)
        enterPage(0);
    _pageStartMs = millis();
}

void PagedStateBase::tick()
{
    if (_finished)
        return;

    const uint32_t now = millis();
    const uint32_t elapsed = now - _pageStartMs;
    if (elapsed >= pageDurationMs(_page))
    {
        if (_page + 1 >= _pageCount)
        {
            _finished = true; // stay on the last page until the controller moves on
            return;
        }
        ++_page;
        enterPage(_page);
        _pageStartMs = now;
        invalidate(Compositor::Transition::Fade);
        return;
    }

    if (pageNeedsRedraw(_page, elapsed))
        invalidate();
}

void PagedStateBase::render(Adafruit_GFX &canvas)
{
    if (_pageCount == 0)
        return;
    renderPage(canvas, _page, millis() - _pageStartMs);
}
