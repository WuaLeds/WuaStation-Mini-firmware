#pragma once

// =============================================================================
//  Display entry point for the whole firmware.
//
//  The LED matrix is driven through the WuaDisplay-LMX library: `WuaDisplay`
//  resolves to WuaDisplayLMX<LMX1P> (the LMX1p 18x7 panoramic panel) because
//  platformio.ini sets -D WUA_BOARD_LMX1P. The library provides the pixel
//  mapping, text, non-blocking scroll, brightness and crossfade effects.
// =============================================================================

#include <WuaDisplay_LMX.h>

#if !defined(WUA_BOARD_LMX1P)
#error "WuaStation-mini drives an LMX1p panel: build with -D WUA_BOARD_LMX1P"
#endif
#if !defined(WUA_ENABLE_EFFECTS)
#error "The screen transitions need the library effects: build with -D WUA_ENABLE_EFFECTS"
#endif

namespace Layout
{
    constexpr int16_t kWidth = LMX1P_WIDTH;   // 18
    constexpr int16_t kHeight = LMX1P_HEIGHT; // 7

    // The classic 5x7 GFX font fills the 7 rows when drawn from y = 0
    // (the library default y = 1 is meant for the 9-row LMX1).
    constexpr int16_t kTextY = 0;
}
