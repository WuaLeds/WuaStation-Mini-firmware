#pragma once

#include <Arduino.h>

// =============================================================================
//  Rotation layout chosen from the settings page:
//
//    time:10,date:4,-weather,ticker:9,timer
//
//  comma-separated screen ids in display order, each with optional seconds.
//  A leading '-' keeps a screen in the list (order and time remembered) but
//  hides it. Screens not listed are hidden too. Unknown ids and duplicates are
//  ignored.
// =============================================================================
enum class ScreenId : uint8_t
{
    Time,
    Date,
    Weather,
    Forecast,
    Indoor,
    Ticker,
    Message,
    Celebration,
    Timer,
    Count
};

struct ScreenSlot
{
    ScreenId id;
    uint16_t seconds; // 0 = the screen's default
    bool enabled;
};

namespace ScreenConfig
{
    constexpr uint8_t kMaxSlots = static_cast<uint8_t>(ScreenId::Count);

    // Parse `spec` into `out` (display order). Returns the number of slots.
    uint8_t parse(const char *spec, ScreenSlot *out, uint8_t max = kMaxSlots);

    // Rewrite `spec` in canonical form ("time:10,-date,..."), dropping what
    // parse() ignores. Falls back to the default layout when no screen is
    // left enabled.
    void normalize(char *spec, size_t size);

    // Every screen once: the ones in `spec` first, in their order, then the
    // rest hidden. For the settings page.
    uint8_t complete(const char *spec, ScreenSlot *out);

    // Write `slots` in canonical form into `out`.
    void format(const ScreenSlot *slots, uint8_t count, char *out, size_t size);

    // Turn one screen on or off in `spec` (rewritten in canonical form).
    // A screen missing from the list is added at the end.
    void setEnabled(char *spec, size_t size, ScreenId id, bool enabled);

    // Whether `spec` shows the screen.
    bool isEnabled(const char *spec, ScreenId id);

    // Setting id ("time") and display name ("Time").
    const char *name(ScreenId id);
    const char *title(ScreenId id);
    bool fromName(const char *name, ScreenId &out);

    // Screen time used when the setting gives none (ms). 0 = the screen ends
    // by itself (scrolling texts). For the ticker: time per symbol.
    uint32_t defaultMs(ScreenId id);

    // Whether the duration counts per ticker symbol instead of per screen.
    bool perSymbol(ScreenId id);
}
