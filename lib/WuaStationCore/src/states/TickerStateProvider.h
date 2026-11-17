#pragma once

#include "config/Config.h"
#include "display/Widgets.h"
#include "providers/ticker/BinanceTickerProvider.h"
#include "states/PagedStateBase.h"

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
//
//  "ticker:N" in the screens setting = N seconds per symbol.
// =============================================================================
class TickerStateProvider : public PagedStateBase
{
public:
    TickerStateProvider(Compositor &compositor, const BinanceTickerProvider &provider);

    bool canBeShown() const override { return _provider.hasData(); }
    void setDurationMs(uint32_t perSymbolMs) override { _perSymbolMs = perSymbolMs; }

    // Shortest text for `price` that fits the panel in TinyFont and stays
    // within Config::kTickerPriceMaxError. Returns false if nothing fits
    // (`out` then holds the full value, to be panned).
    static bool formatPrice(double price, char *out, size_t size);
    // "+2.3%", "-12%", "123%" ... (always fits).
    static void formatChange(float percent, char *out, size_t size);
    // Trend / 24 h change color of a quote's price.
    static uint16_t priceColor(const TickerQuote &quote);

protected:
    uint8_t beginPages() override;
    void enterPage(uint8_t page) override;
    uint32_t pageDurationMs(uint8_t page) const override;
    void renderPage(Adafruit_GFX &canvas, uint8_t page, uint32_t elapsedMs) override;
    bool pageNeedsRedraw(uint8_t page, uint32_t elapsedMs) override;

private:
    static constexpr uint8_t kPagesPerSymbol = 3; // name, price, change

    const BinanceTickerProvider &_provider;
    TickerSnapshot _snapshot{};
    uint8_t _symbols[Config::kMaxTickers] = {}; // snapshot indexes with a price
    uint32_t _perSymbolMs = Config::kTickerSymbolMs;

    Widgets::Label _label{};
    int16_t _drawnOffset = 0;
};
