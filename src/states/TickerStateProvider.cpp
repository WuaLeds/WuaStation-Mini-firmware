#include "states/TickerStateProvider.h"

#include <math.h>
#include "config/Config.h"
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
    : StaticStateBase(compositor, "ticker", 0), _provider(provider)
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

// -----------------------------------------------------------------------------
//  Pages
// -----------------------------------------------------------------------------
bool TickerStateProvider::selectQuote(uint8_t from)
{
    for (uint8_t i = from; i < _snapshot.count; ++i)
    {
        if (_snapshot.quotes[i].hasPrice)
        {
            _quote = i;
            return true;
        }
    }
    return false;
}

bool TickerStateProvider::nextPage()
{
    switch (_page)
    {
    case Page::Name:
        _page = Page::Price;
        break;
    case Page::Price:
        _page = Page::Change;
        break;
    case Page::Change:
        if (!selectQuote(_quote + 1))
            return false;
        _page = Page::Name;
        break;
    }
    preparePage();
    return true;
}

void TickerStateProvider::preparePage()
{
    const TickerQuote &q = _snapshot.quotes[_quote];
    uint32_t baseDurationMs = Config::kTickerNameMs;

    switch (_page)
    {
    case Page::Name:
        strlcpy(_text, q.base[0] ? q.base : q.symbol, sizeof(_text));
        _color = Theme::kTickerName;
        // Classic 5x7 font when it fits (3 letters), else the compact one.
        if (Widgets::textWidth(Widgets::Font::Classic, _text) <= Layout::kWidth)
            _font = Widgets::Font::Classic;
        else if (Widgets::textWidth(Widgets::Font::Tiny, _text) <= Layout::kWidth)
            _font = Widgets::Font::Tiny;
        else
            _font = Widgets::Font::Classic; // too long either way: pan it
        baseDurationMs = Config::kTickerNameMs;
        break;

    case Page::Price:
        formatPrice(q.last, _text, sizeof(_text));
        _font = Widgets::Font::Tiny;
        switch (q.trend)
        {
        case PriceTrend::Up:
            _color = Theme::kTickerUp;
            break;
        case PriceTrend::Down:
            _color = Theme::kTickerDown;
            break;
        case PriceTrend::Flat:
            _color = Theme::kTickerFlat;
            break;
        case PriceTrend::Unknown:
        default:
            _color = q.changePct >= 0.0f ? Theme::kTickerUp : Theme::kTickerDown;
            break;
        }
        baseDurationMs = Config::kTickerPriceMs;
        break;

    case Page::Change:
        formatChange(q.changePct, _text, sizeof(_text));
        _font = Widgets::Font::Tiny;
        _color = q.changePct >= 0.0f ? Theme::kTickerUp : Theme::kTickerDown;
        baseDurationMs = Config::kTickerChangeMs;
        break;
    }

    _textWidth = Widgets::textWidth(_font, _text);
    _pageStartMs = millis();
    _shownOffset = 0;

    // A page wider than the panel stays long enough to pan all the way.
    const int16_t overflow = _textWidth - Layout::kWidth;
    const uint32_t panMs = overflow > 0
                               ? 2 * Config::kTickerPanHoldMs + static_cast<uint32_t>(overflow) * Config::kScrollSpeedMs
                               : 0;
    _pageDurationMs = panMs > baseDurationMs ? panMs : baseDurationMs;
}

int16_t TickerStateProvider::panOffset(uint32_t elapsedMs) const
{
    const int16_t overflow = _textWidth - Layout::kWidth;
    if (overflow <= 0 || elapsedMs <= Config::kTickerPanHoldMs)
        return 0;
    const uint32_t offset = (elapsedMs - Config::kTickerPanHoldMs) / Config::kScrollSpeedMs;
    return offset >= static_cast<uint32_t>(overflow) ? overflow : static_cast<int16_t>(offset);
}

// -----------------------------------------------------------------------------
//  StaticStateBase hooks
// -----------------------------------------------------------------------------
void TickerStateProvider::onShow()
{
    // Freeze one snapshot per visit so a refresh never reshuffles the pages.
    _provider.read(_snapshot);
    _page = Page::Name;
    _finished = !selectQuote(0);
    if (_finished)
    {
        _text[0] = '\0';
        _textWidth = 0;
        return;
    }
    preparePage();
}

void TickerStateProvider::tick()
{
    if (_finished)
        return;

    const uint32_t elapsed = millis() - _pageStartMs;
    if (elapsed >= _pageDurationMs)
    {
        if (nextPage())
            invalidate(Compositor::Transition::Fade);
        else
            _finished = true; // every symbol shown: back to the rotation
        return;
    }

    if (panOffset(elapsed) != _shownOffset)
        invalidate();
}

void TickerStateProvider::render(Adafruit_GFX &canvas)
{
    if (_text[0] == '\0')
        return;
    _shownOffset = panOffset(millis() - _pageStartMs);
    const int16_t x = _textWidth <= Layout::kWidth ? (Layout::kWidth - _textWidth) / 2 : -_shownOffset;
    Widgets::drawText(canvas, _font, x, _text, _color.to565());
}
