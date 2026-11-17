#pragma once

#include "states/StaticStateBase.h"

// =============================================================================
//  PagedStateBase -- a static screen made of several fixed pages (e.g. ticker:
//  name -> price -> change, forecast: max -> min -> rain -> UV -> air).
//
//  The page list is built on entry (beginPages), pages crossfade into each
//  other and the state finishes after the last one. With a screen duration
//  (> 0) the controller ends it by time instead, so pages that share that
//  duration simply stay on the last page until then.
// =============================================================================
class PagedStateBase : public StaticStateBase
{
public:
    PagedStateBase(Compositor &compositor, const char *id, uint32_t durationMs, uint8_t priority = 0)
        : StaticStateBase(compositor, id, durationMs, priority)
    {
    }

    bool isFinished() const override { return _finished; }

protected:
    // Snapshot the data for this visit and return the number of pages
    // (0 = nothing to show, the state finishes at once).
    virtual uint8_t beginPages() = 0;

    // A page becomes current: prepare its texts / colors.
    virtual void enterPage(uint8_t page) { (void)page; }

    virtual uint32_t pageDurationMs(uint8_t page) const = 0;

    // Draw `page` (`elapsedMs` since it became current).
    virtual void renderPage(Adafruit_GFX &canvas, uint8_t page, uint32_t elapsedMs) = 0;

    // Time-driven content (e.g. a panning label): return true to redraw.
    virtual bool pageNeedsRedraw(uint8_t page, uint32_t elapsedMs)
    {
        (void)page;
        (void)elapsedMs;
        return false;
    }

    uint8_t pageCount() const { return _pageCount; }

private:
    void onShow() final;
    void tick() final;
    void render(Adafruit_GFX &canvas) final;

    uint8_t _pageCount = 0;
    uint8_t _page = 0;
    uint32_t _pageStartMs = 0;
    bool _finished = false;
};
