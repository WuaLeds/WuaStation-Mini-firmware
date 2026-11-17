#include "states/TickerStateProvider.h"

#include <math.h>
#include "config/Theme.h"
#include "display/TinyFont.h"

namespace
{
    bool fitsPanel(const char *text)
    {
        return TinyFont::textWidth(text) <= Layout::kWidth;
    }

    // Does `text` (times `scale`) still represent `value` closely enough?
    bool closeEnough(const char *text, double value, double scale)
    {
        const double shown = strtod(text, nullptr) * scale;
        if (value == 0.0)
            return shown == 0.0;
        return fabs(shown - value) / fabs(value) <= Config::kTickerPriceMaxError;
    }

    bool accept(const char *candidate, double value, double scale, char *out, size_t size)
    {
        if (!fitsPanel(candidate) || !closeEnough(candidate, value, scale))
            return false;
        strlcpy(out, candidate, size);
        return true;
    }
}

TickerStateProvider::TickerStateProvider(Compositor &compositor, const BinanceTickerProvider &provider)
    : PagedStateBase(compositor, "ticker", 0), _provider(provider)
{
}

// -----------------------------------------------------------------------------
//  Formatting
// -----------------------------------------------------------------------------
bool TickerStateProvider::formatPrice(double price, char *out, size_t size)
{
    char buf[24];

    // 1) Plain number with as many decimals as fit: "3457", "145.7", "0.457"
    for (int decimals = 6; decimals >= 0; --decimals)
    {
        snprintf(buf, sizeof(buf), "%.*f", decimals, price);
        if (accept(buf, price, 1.0, out, size))
            return true;
    }

    // 2) Below 1, drop the leading zero: ".0457"
    if (price > 0.0 && price < 1.0)
    {
        for (int decimals = 6; decimals >= 1; --decimals)
        {
            snprintf(buf, sizeof(buf), "%.*f", decimals, price);
            if (accept(buf + 1, price, 1.0, out, size))
                return true;
        }
    }

    // 3) Abbreviated: "97.2K", "1.2M"
    static const struct
    {
        double scale;
        char suffix;
    } kUnits[] = {{1e3, 'K'}, {1e6, 'M'}, {1e9, 'B'}};
    for (const auto &unit : kUnits)
    {
        if (fabs(price) < unit.scale)
            break;
        for (int decimals = 2; decimals >= 0; --decimals)
        {
            snprintf(buf, sizeof(buf), "%.*f%c", decimals, price / unit.scale, unit.suffix);
            if (accept(buf, price, unit.scale, out, size))
                return true;
        }
    }

    // 4) Nothing fits (e.g. 0.00001234): full value, trailing zeros trimmed.
    snprintf(buf, sizeof(buf), price >= 1.0 ? "%.2f" : "%.8f", price);
    if (strchr(buf, '.'))
    {
        size_t len = strlen(buf);
        while (len > 0 && buf[len - 1] == '0')
            buf[--len] = '\0';
        if (len > 0 && buf[len - 1] == '.')
            buf[--len] = '\0';
    }
    strlcpy(out, buf, size);
    return false;
}

void TickerStateProvider::formatChange(float percent, char *out, size_t size)
{
    static const char *const kFormats[] = {"%+.2f%%", "%+.1f%%", "%+.0f%%"};
    for (const char *format : kFormats)
    {
        snprintf(out, size, format, percent);
        if (fitsPanel(out))
            return;
    }
    // Large moves: drop the sign, the color already tells the direction.
    snprintf(out, size, "%.0f%%", fabsf(percent));
}

uint16_t TickerStateProvider::priceColor(const TickerQuote &quote)
{
    switch (quote.trend)
    {
    case PriceTrend::Up: return Theme::kTickerUp.to565();
    case PriceTrend::Down: return Theme::kTickerDown.to565();
    case PriceTrend::Flat: return Theme::kTickerFlat.to565();
    case PriceTrend::Unknown:
    default: return (quote.changePct >= 0.0f ? Theme::kTickerUp : Theme::kTickerDown).to565();
    }
}

// -----------------------------------------------------------------------------
//  Pages
// -----------------------------------------------------------------------------
uint8_t TickerStateProvider::beginPages()
{
    // Freeze one snapshot per visit so a refresh never reshuffles the pages.
    _provider.read(_snapshot);
    uint8_t symbols = 0;
    for (uint8_t i = 0; i < _snapshot.count; ++i)
    {
        if (_snapshot.quotes[i].hasPrice)
            _symbols[symbols++] = i;
    }
    return static_cast<uint8_t>(symbols * kPagesPerSymbol);
}

void TickerStateProvider::enterPage(uint8_t page)
{
    const TickerQuote &q = _snapshot.quotes[_symbols[page / kPagesPerSymbol]];
    char text[24];

    switch (page % kPagesPerSymbol)
    {
    case 0: // name
    {
        const char *name = q.base[0] ? q.base : q.symbol;
        Widgets::makeLabel(_label, name, Theme::kTickerName.to565(), Widgets::bestFont(name));
        break;
    }
    case 1: // price
        formatPrice(q.last, text, sizeof(text));
        Widgets::makeLabel(_label, text, priceColor(q), Widgets::Font::Tiny);
        break;
    default: // 24 h change
        formatChange(q.changePct, text, sizeof(text));
        Widgets::makeLabel(_label, text, (q.changePct >= 0.0f ? Theme::kTickerUp : Theme::kTickerDown).to565(),
                           Widgets::Font::Tiny);
        break;
    }
    _drawnOffset = 0;
}

uint32_t TickerStateProvider::pageDurationMs(uint8_t page) const
{
    static const uint32_t kWeights[kPagesPerSymbol] = {Config::kTickerNameMs, Config::kTickerPriceMs,
                                                       Config::kTickerChangeMs};
    // Same proportions as the defaults, scaled to the per-symbol time.
    const uint32_t base = kWeights[page % kPagesPerSymbol] * _perSymbolMs / Config::kTickerSymbolMs;
    const uint32_t pan = Widgets::panDurationMs(_label); // only the current page is asked
    return pan > base ? pan : base;
}

void TickerStateProvider::renderPage(Adafruit_GFX &canvas, uint8_t page, uint32_t elapsedMs)
{
    (void)page;
    _drawnOffset = Widgets::panOffset(_label, elapsedMs);
    Widgets::drawLabel(canvas, _label, elapsedMs);
}

bool TickerStateProvider::pageNeedsRedraw(uint8_t page, uint32_t elapsedMs)
{
    (void)page;
    return Widgets::panOffset(_label, elapsedMs) != _drawnOffset;
}
