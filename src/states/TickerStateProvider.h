#pragma once

#include "providers/ticker/BinanceTickerProvider.h"
#include "states/ScrollTextStateBase.h"

// =============================================================================
//  Market ticker: scrolls one pass per symbol, e.g. "BTC/USDT 97234 +2.3%".
//  Color follows the trend vs. the previous sample (green up, red down, as in
//  WuaDisplay-r1); on the first sample it follows the 24 h change.
// =============================================================================
class TickerStateProvider : public ScrollTextStateBase
{
public:
    TickerStateProvider(Compositor &compositor, const BinanceTickerProvider &provider)
        : ScrollTextStateBase(compositor, "ticker", 0, 1), _provider(provider) {}

    bool canBeShown() const override { return _provider.hasData(); }
    void onEnter(WuaDisplay &display) override;

    static void formatPrice(double price, char *out, size_t size);

protected:
    bool nextPass(String &text, Rgb &color) override;
    uint8_t passCount() const override { return _validCount; }

private:
    const BinanceTickerProvider &_provider;
    TickerSnapshot _snapshot{};
    uint8_t _validCount = 0;
    uint8_t _cursor = 0;
};
