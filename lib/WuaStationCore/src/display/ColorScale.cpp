#include "display/ColorScale.h"

namespace ColorScale
{
    namespace
    {
        struct Stop
        {
            float value;
            Rgb color;
        };

        // clang-format off
        const Stop kTemperature[] = {
            {-10.0f, {90, 0, 255}},
            {0.0f, {0, 90, 255}},
            {10.0f, {0, 200, 220}},
            {18.0f, {80, 220, 40}},
            {24.0f, {255, 190, 0}},
            {30.0f, {255, 90, 0}},
            {36.0f, {255, 0, 0}},
        };
        // clang-format on

        uint8_t lerp(uint8_t a, uint8_t b, float t)
        {
            return static_cast<uint8_t>(a + (static_cast<float>(b) - a) * t + 0.5f);
        }
    }

    Rgb temperature(float celsius)
    {
        constexpr size_t n = sizeof(kTemperature) / sizeof(kTemperature[0]);
        if (celsius <= kTemperature[0].value)
            return kTemperature[0].color;
        for (size_t i = 1; i < n; ++i)
        {
            if (celsius <= kTemperature[i].value)
            {
                const Stop &a = kTemperature[i - 1];
                const Stop &b = kTemperature[i];
                const float t = (celsius - a.value) / (b.value - a.value);
                return {lerp(a.color.r, b.color.r, t), lerp(a.color.g, b.color.g, t), lerp(a.color.b, b.color.b, t)};
            }
        }
        return kTemperature[n - 1].color;
    }

    Rgb uvIndex(float uv)
    {
        if (uv < 3.0f)
            return {0, 200, 60};
        if (uv < 6.0f)
            return {255, 210, 0};
        if (uv < 8.0f)
            return {255, 120, 0};
        if (uv < 11.0f)
            return {255, 20, 20};
        return {180, 40, 255};
    }

    // clang-format off
    Rgb airQuality(uint16_t aqi, bool european)
    {
        if (european)
        {
            if (aqi <= 20) return {80, 240, 230};  // good
            if (aqi <= 40) return {80, 204, 120};  // fair
            if (aqi <= 60) return {240, 230, 65};  // moderate
            if (aqi <= 80) return {255, 80, 80};   // poor
            if (aqi <= 100) return {170, 0, 50};   // very poor
            return {125, 33, 129};                 // extremely poor
        }
        if (aqi <= 50) return {0, 220, 60};        // good
        if (aqi <= 100) return {255, 220, 0};      // moderate
        if (aqi <= 150) return {255, 120, 0};      // unhealthy for sensitive groups
        if (aqi <= 200) return {255, 20, 20};      // unhealthy
        if (aqi <= 300) return {160, 40, 200};     // very unhealthy
        return {150, 0, 40};                       // hazardous
    }
    // clang-format on
}
