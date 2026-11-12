// =============================================================================
//  WuaStation-mini firmware -- entry point.
//  All the logic lives in app/App (normal mode) or bench/Benchmark (c3_bench).
// =============================================================================
#include <Arduino.h>

#if defined(WUA_ENABLE_BENCHMARK)

#include "bench/Benchmark.h"

void setup() { Benchmark::setup(); }
void loop() { Benchmark::loop(); }

#else

#include "app/App.h"

static App app;

void setup() { app.setup(); }
void loop() { app.loop(); }

#endif
