#include "states/TickerStateProvider.h"

void TickerStateProvider::formatPrice(double price, char *out, size_t size)
{
    // Adaptive precision: 97234 / 2.4512 / 0.000012
    if (price >= 1000.0)
        snprintf(out, size, "%.0f", price);
    else if (price >= 1.0)
        snprintf(out, size, "%.2f", price);
    else if (price >= 0.01)
        snprintf(out, size, "%.4f", price);
    else
        snprintf(out, size, "%.8f", price);
}

void TickerStateProvider::onEnter(WuaDisplay &display)
{
    // Freeze one snapshot per visit: every symbol scrolls once.
    _provider.read(_snapshot);
    _validCount = 0;
    for (uint8_t i = 0; i < _snapshot.count; ++i)
    {
        if (_snapshot.quotes[i].hasPrice)
            ++_validCount;
    }
    _cursor = 0;
    ScrollTextStateBase::onEnter(display);
}

bool TickerStateProvider::nextPass(String &text, Rgb &color)
{
    while (_cursor < _snapshot.count)
    {
        const TickerQuote &q = _snapshot.quotes[_cursor++];
        if (!q.hasPrice)
            continue;

        char price[24];
        formatPrice(q.last, price, sizeof(price));
        char buf[64];
        snprintf(buf, sizeof(buf), "%s %s %+.1f%%", q.pair, price, q.changePct);
        text = buf;

        switch (q.trend)
        {
        case PriceTrend::Up:
            color = Theme::kTickerUp;
            break;
        case PriceTrend::Down:
            color = Theme::kTickerDown;
            break;
        case PriceTrend::Flat:
            color = Theme::kTickerFlat;
            break;
        case PriceTrend::Unknown:
        default:
            color = q.changePct >= 0.0f ? Theme::kTickerUp : Theme::kTickerDown;
            break;
        }
        return true;
    }
    return false;
}
