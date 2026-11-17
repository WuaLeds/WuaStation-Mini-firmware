#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <atomic>
#include "core/SharedState.h"
#include "net/NetWorker.h"
#include "providers/ticker/TickerTypes.h"

// =============================================================================
//  Binance public market data (no API key).
//
//    once per symbol : /api/v3/exchangeInfo?symbol=X  -> "BASE/QUOTE" name
//    every poll      : /api/v3/ticker/24hr?symbol=X   -> last/open/high/low/%
//
//  api.binance.com is tried first; data-api.binance.vision (Binance's public
//  market-data mirror) is the fallback when the main host is geo-blocked.
//  Runs as an INetJob in the network task.
// =============================================================================
class BinanceTickerProvider : public INetJob
{
public:
    // "BTCUSDT,ETHUSDT" (already normalized by SettingsStore). Call before
    // NetWorker::begin().
    void configure(const char *symbolsCsv);

    const char *name() const override { return "binance"; }
    bool enabled() const override { return _work.count > 0; }
    uint32_t run() override;

    // Main-loop side.
    uint32_t read(TickerSnapshot &out) const { return _shared.read(out); }
    uint32_t version() const { return _shared.version(); }
    bool hasData() const { return _hasData.load(); }

private:
    bool fetchMeta(TickerQuote &q, int &code);
    bool fetchQuote(TickerQuote &q, int &code);
    bool request(const String &pathAndQuery, JsonDocument &doc, const JsonDocument *filter, int &code);

    TickerSnapshot _work{}; // owned by the network task
    bool _metaDone[Config::kMaxTickers] = {};
    uint8_t _host = 0;
    SharedState<TickerSnapshot> _shared;
    std::atomic<bool> _hasData{false};
};
