// Only compiled into the c3_bench environment.
#if defined(WUA_ENABLE_BENCHMARK)

#include "bench/Benchmark.h"

#include <WiFi.h>
#include "bench/FpsMeter.h"
#include "board/Board.h"
#include "config/Theme.h"
#include "core/Logger.h"
#include "display/Display.h"
#include "display/TinyFont.h"

namespace
{
    WuaDisplay display;
    FpsMeter fps(1000, 4); // 1 s window, 4-sample moving average
    uint16_t lastFps = 0;
    uint16_t frameCounter = 0;

    void drawFrame()
    {
        auto &panel = display.panel();
        display.clear();

        // Moving pixel along the bottom row: proves every frame is pushed.
        panel.drawPixel(frameCounter % Layout::kWidth, Layout::kHeight - 1, Theme::kSplash.to565());

        if (lastFps > 0)
        {
            char buf[8];
            snprintf(buf, sizeof(buf), "%u", lastFps);
            TinyFont::drawCentered(panel, buf, Theme::kClock.to565());
        }
        display.flush();
        ++frameCounter;
    }
}

namespace Benchmark
{
    void setup()
    {
        Serial.begin(115200);
        Board::init();
        WiFi.mode(WIFI_OFF); // pure display benchmark

        display.begin();
        display.setBrightness(32);
        LOG_INFO("Benchmark mode: %dx%d LMX1p", Layout::kWidth, Layout::kHeight);
    }

    void loop()
    {
        display.update();
        drawFrame();
        fps.frame();
        if (fps.hasNewValue())
        {
            lastFps = fps.getAverageFps();
            fps.clearUpdated();
            LOG_INFO("FPS: %u (instant %u)", lastFps, fps.getFps());
        }
    }
}

#endif // WUA_ENABLE_BENCHMARK
