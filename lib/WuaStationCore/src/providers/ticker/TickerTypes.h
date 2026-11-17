#pragma once

#include <Arduino.h>
#include "config/Config.h"

enum class PriceTrend : uint8_t
{
    Unknown,
    Up,
    Down,
    Flat
};

enum class QuoteStatus : uint8_t
{
    Pending, // not fetched yet
    Ok,
    Invalid, // symbol rejected by the exchange
    Error    // last request failed (previous values kept)
};

// One market quote. Plain data: shared between tasks via SharedState.
struct TickerQuote
{
    char symbol[16]; // exchange symbol, e.g. "BTCUSDT"
    char base[12];   // base asset, e.g. "BTC"
    char pair[20];   // display name, e.g. "BTC/USDT"
    double last;
    double open;
    double high;
    double low;
    float changePct;  // 24 h change, percent
    PriceTrend trend; // vs. the previous sample
    QuoteStatus status;
    bool hasPrice;
    uint32_t updatedMs;
};

struct TickerSnapshot
{
    uint8_t count;
    TickerQuote quotes[Config::kMaxTickers];
};
