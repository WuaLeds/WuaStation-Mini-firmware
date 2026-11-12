#pragma once

#include <Arduino.h>
#include <stdarg.h>

// =============================================================================
//  Minimal logger. Enabled only when WUA_ENABLE_DEBUG is defined (see
//  platformio.ini); otherwise every LOG_* call compiles to nothing.
//  Safe to call from the network task: the buffer lives on the caller's stack.
// =============================================================================

#if defined(WUA_ENABLE_DEBUG)

inline void wua_log(const char *level, const char *fmt, ...)
{
    char buffer[200];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    Serial.printf("[%7lu][%s] %s\n", static_cast<unsigned long>(millis()), level, buffer);
}

#define LOG_INFO(fmt, ...) wua_log("INFO ", fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) wua_log("WARN ", fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) wua_log("ERROR", fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) wua_log("DEBUG", fmt, ##__VA_ARGS__)

#else

#define LOG_INFO(fmt, ...) ((void)0)
#define LOG_WARN(fmt, ...) ((void)0)
#define LOG_ERROR(fmt, ...) ((void)0)
#define LOG_DEBUG(fmt, ...) ((void)0)

#endif
