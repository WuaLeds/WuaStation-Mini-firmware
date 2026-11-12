#pragma once

#include <Arduino.h>

// 24-bit color used across the UI. Content is drawn at full range; the
// perceived level is set once by the master brightness (WuaDisplay).
struct Rgb
{
    uint8_t r;
    uint8_t g;
    uint8_t b;

    constexpr uint16_t to565() const
    {
        return static_cast<uint16_t>(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
    }
};

namespace Theme
{
    constexpr Rgb kBlack{0, 0, 0};

    constexpr Rgb kSplash{255, 60, 160};
    constexpr Rgb kClock{0, 170, 255};
    constexpr Rgb kClockColon{0, 110, 200};
    constexpr Rgb kDate{255, 140, 0};
    constexpr Rgb kWeatherTemp{255, 170, 40};
    constexpr Rgb kIndoorTemp{255, 110, 60};
    constexpr Rgb kHumidity{60, 160, 255};
    constexpr Rgb kPressure{190, 120, 255};

    constexpr Rgb kTickerUp{0, 230, 60};
    constexpr Rgb kTickerDown{255, 40, 30};
    constexpr Rgb kTickerFlat{210, 210, 210};

    constexpr Rgb kMessage{200, 80, 255};
    constexpr Rgb kCelebration{255, 60, 180};
    constexpr Rgb kAlert{255, 40, 30};
    constexpr Rgb kInfo{0, 210, 130};
    constexpr Rgb kAlarm{255, 50, 0};
    constexpr Rgb kPortal{0, 200, 255};

    constexpr Rgb kWifiConnecting{0, 170, 255};
    constexpr Rgb kWifiSyncing{0, 220, 80};
    constexpr Rgb kWifiOffline{90, 90, 100};
    constexpr Rgb kStatusDots{200, 200, 200};
    constexpr Rgb kStatusError{255, 30, 20};
}
