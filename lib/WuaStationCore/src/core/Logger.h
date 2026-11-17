#pragma once

#include <Arduino.h>
#include <stdarg.h>

// =============================================================================
//  Minimal logger with three back ends:
//    * WUA_LOG_HOOK (ESPHome build): every message goes to wua_log_write(),
//      implemented by the wuastation component, which forwards it to the
//      ESPHome logger (serial, ESPHome dashboard, Home Assistant).
//    * WUA_ENABLE_DEBUG: printed on Serial.
//    * otherwise every LOG_* call compiles to nothing.
//  Safe to call from the network task: the buffer lives on the caller's stack.
// =============================================================================

enum class LogLevel : uint8_t
{
    Error,
    Warn,
    Info,
    Debug
};

#if defined(WUA_LOG_HOOK)

// Provided by the host (components/wuastation).
void wua_log_write(LogLevel level, const char *message);

inline void wua_log(LogLevel level, const char *fmt, ...)
{
    char buffer[200];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    wua_log_write(level, buffer);
}

#define LOG_INFO(fmt, ...) wua_log(LogLevel::Info, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) wua_log(LogLevel::Warn, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) wua_log(LogLevel::Error, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) wua_log(LogLevel::Debug, fmt, ##__VA_ARGS__)

#elif defined(WUA_ENABLE_DEBUG)

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
