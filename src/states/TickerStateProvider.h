#pragma once

#include "config/Theme.h"
#include "display/Widgets.h"
#include "providers/ticker/BinanceTickerProvider.h"
#include "states/StaticStateBase.h"

// =============================================================================
//  Market ticker as fixed pages (no scrolling), for every configured symbol:
//
//    1. name    "BTC"     amber
//    2. price   "97.2K"   green / red / gray by trend vs. the previous sample
//    3. change  "+2.3%"   green / red by the 24 h change
//
//  Pages crossfade into each other. Everything is fitted to the 18 px width:
//  3-letter names use the classic font, longer ones TinyFont; prices get as
//  many decimals as fit and are abbreviated (K / M) when needed. A page that
//  still does not fit (e.g. "MATIC") pans slowly instead of scrolling away.
// =============================================================================
class TickerStateProvider : public StaticStateBase
{
public:
    TickerStateProvider(Compositor &compositor, const BinanceTickerProvider &provider);

    bool canBeShown() const override { return _provider.hasData(); }
    bool isFinished() const override { return _finished; }

    // Shortest text for `price` that fits the panel in TinyFont and stays
    // within Config::kTickerPriceMaxError. Returns false if nothing fits
    // (`out` then holds the full value, to be panned).
    static bool formatPrice(double price, char *out, size_t size);
    // "+2.3%", "-12%", "123%" ... (always fits).
    static void formatChange(float percent, char *out, size_t size);

protected:
    void onShow() override;
    void tick() override;
    void render(Adafruit_GFX &canvas) override;

private:
    enum class Page : uint8_t
    {
        Name,
        Price,
        Change
    };

    bool selectQuote(uint8_t from); // first quote with a price at index >= from
    bool nextPage();                // false when every symbol was shown
    void preparePage();
    int16_t panOffset(uint32_t elapsedMs) const;

    const BinanceTickerProvider &_provider;
    TickerSnapshot _snapshot{};
    uint8_t _quote = 0;
    Page _page = Page::Name;
    bool _finished = false;

    // Current page
    char _text[24] = {};
    Widgets::Font _font = Widgets::Font::Tiny;
    Rgb _color = Theme::kTickerFlat;
    int16_t _textWidth = 0;
    int16_t _shownOffset = 0;
    uint32_t _pageStartMs = 0;
    uint32_t _pageDurationMs = 0;
};
