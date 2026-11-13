#include "providers/ticker/BinanceTickerProvider.h"

#include <HTTPClient.h>
#include "core/Logger.h"
#include "net/HttpJson.h"

namespace
{
    const char *const kHosts[] = {"api.binance.com", "data-api.binance.vision"};
    constexpr uint8_t kHostCount = sizeof(kHosts) / sizeof(kHosts[0]);

    double toDouble(JsonVariantConst v)
    {
        // Binance sends prices as strings ("97234.12000000").
        const char *s = v.as<const char *>();
        return s ? strtod(s, nullptr) : v.as<double>();
    }

    // Best-effort base asset until exchangeInfo answers: "BTCUSDT" -> "BTC".
    void guessBase(const char *symbol, char *out, size_t size)
    {
        static const char *const kQuotes[] = {"FDUSD", "USDT", "USDC", "BUSD", "TUSD", "EUR",
                                              "TRY", "BRL", "ARS", "BTC", "ETH", "BNB"};
        const size_t len = strlen(symbol);
        for (const char *quote : kQuotes)
        {
            const size_t qlen = strlen(quote);
            if (len > qlen && strcmp(symbol + len - qlen, quote) == 0)
            {
                const size_t blen = len - qlen < size - 1 ? len - qlen : size - 1;
                memcpy(out, symbol, blen);
                out[blen] = '\0';
                return;
            }
        }
        strlcpy(out, symbol, size);
    }
}

void BinanceTickerProvider::configure(const char *symbolsCsv)
{
    memset(&_work, 0, sizeof(_work));
    const char *p = symbolsCsv;
    while (*p && _work.count < Config::kMaxTickers)
    {
        const char *comma = strchr(p, ',');
        const size_t len = comma ? static_cast<size_t>(comma - p) : strlen(p);
        if (len > 0 && len < sizeof(TickerQuote::symbol))
        {
            TickerQuote &q = _work.quotes[_work.count++];
            memcpy(q.symbol, p, len);
            q.symbol[len] = '\0';
            strlcpy(q.pair, q.symbol, sizeof(q.pair));
            guessBase(q.symbol, q.base, sizeof(q.base));
            q.status = QuoteStatus::Pending;
        }
        if (!comma)
            break;
        p = comma + 1;
    }
    _shared.write(_work);
    LOG_INFO("Ticker: %u symbol(s) configured", _work.count);
}

bool BinanceTickerProvider::request(const String &pathAndQuery, JsonDocument &doc,
                                    const JsonDocument &filter, int &code)
{
    for (uint8_t attempt = 0; attempt < kHostCount; ++attempt)
    {
        const uint8_t host = (_host + attempt) % kHostCount;
        doc.clear();
        code = HttpJson::get(String("https://") + kHosts[host] + pathAndQuery, doc, &filter);
        if (code == HTTP_CODE_OK)
        {
            _host = host; // stick to the host that works
            return true;
        }
        if (code == 400)
            return false; // bad symbol: same answer on every host
    }
    return false;
}

bool BinanceTickerProvider::fetchMeta(TickerQuote &q, int &code)
{
    JsonDocument filter;
    filter["symbols"][0]["baseAsset"] = true;
    filter["symbols"][0]["quoteAsset"] = true;

    JsonDocument doc;
    if (!request(String("/api/v3/exchangeInfo?symbol=") + q.symbol, doc, filter, code))
        return false;

    const char *base = doc["symbols"][0]["baseAsset"];
    const char *quote = doc["symbols"][0]["quoteAsset"];
    if (!base || !quote)
        return false;
    strlcpy(q.base, base, sizeof(q.base));
    snprintf(q.pair, sizeof(q.pair), "%s/%s", base, quote);
    return true;
}

bool BinanceTickerProvider::fetchQuote(TickerQuote &q, int &code)
{
    JsonDocument filter;
    filter["lastPrice"] = true;
    filter["openPrice"] = true;
    filter["highPrice"] = true;
    filter["lowPrice"] = true;
    filter["priceChangePercent"] = true;

    JsonDocument doc;
    if (!request(String("/api/v3/ticker/24hr?symbol=") + q.symbol, doc, filter, code))
        return false;
    if (doc["lastPrice"].isNull())
        return false;

    const double last = toDouble(doc["lastPrice"]);

    // Trend vs. the previous sample (same coloring rule as WuaDisplay-r1).
    constexpr double kEps = 1e-9;
    if (!q.hasPrice)
        q.trend = PriceTrend::Unknown;
    else if (last > q.last + kEps)
        q.trend = PriceTrend::Up;
    else if (last < q.last - kEps)
        q.trend = PriceTrend::Down;
    else
        q.trend = PriceTrend::Flat;

    q.last = last;
    q.open = toDouble(doc["openPrice"]);
    q.high = toDouble(doc["highPrice"]);
    q.low = toDouble(doc["lowPrice"]);
    q.changePct = static_cast<float>(toDouble(doc["priceChangePercent"]));
    q.hasPrice = true;
    q.updatedMs = millis();
    return true;
}

uint32_t BinanceTickerProvider::run()
{
    bool anyError = false;
    bool anyOk = false;

    for (uint8_t i = 0; i < _work.count; ++i)
    {
        TickerQuote &q = _work.quotes[i];
        if (q.status == QuoteStatus::Invalid)
            continue;

        int code = 0;
        if (!_metaDone[i])
        {
            _metaDone[i] = fetchMeta(q, code);
            if (!_metaDone[i] && code == 400)
            {
                LOG_WARN("Ticker %s rejected by Binance", q.symbol);
                q.status = QuoteStatus::Invalid;
                continue;
            }
            // Network error: keep the raw symbol as name and retry next time.
        }

        if (fetchQuote(q, code))
        {
            q.status = QuoteStatus::Ok;
            anyOk = true;
            LOG_DEBUG("Ticker %s = %.8g (%+.2f%%)", q.pair, q.last, q.changePct);
        }
        else if (code == 400)
        {
            q.status = QuoteStatus::Invalid;
        }
        else
        {
            q.status = q.hasPrice ? QuoteStatus::Ok : QuoteStatus::Error; // keep last price
            anyError = true;
        }
    }

    _shared.write(_work);
    if (anyOk)
        _hasData.store(true);
    return anyError ? Config::kTickerRetryMs : Config::kTickerPollMs;
}
