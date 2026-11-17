// =============================================================================
//  WuaStation-mini -- LED pipeline benchmark (PlatformIO env c3_bench).
//
//  The product firmware is built with ESPHome from wuastation-mini.yaml; the
//  firmware core lives in lib/WuaStationCore. This entry point only measures
//  the FPS of the display stack (WiFi off).
// =============================================================================
#include <Arduino.h>

#include "bench/Benchmark.h"

void setup()
{
    Benchmark::setup();
}

void loop()
{
    Benchmark::loop();
}
