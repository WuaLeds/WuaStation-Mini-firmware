#pragma once

#include <Arduino.h>

// =============================================================================
//  Firmware configuration: compile-time defaults and tuning constants.
//  Everything the user can change at runtime (timezone, city, tickers...) is
//  only the *default* here; the live value is stored in NVS (core/Settings)
//  and edited from the WiFi portal.
// =============================================================================

#define WUA_FIRMWARE_VERSION "1.0.0"

namespace Config
{
    // ---- Identity / WiFi ----------------------------------------------------
    constexpr const char *kDeviceName = "WuaStation";     // AP: "WuaStation-XXXX"
    constexpr const char *kHostnamePrefix = "wuastation"; // "wuastation-xxxx.local"
    constexpr const char *kPortalPassword = "WuaLeds2025"; // >= 8 chars, "" = open AP

    constexpr uint32_t kWifiConnectTimeoutMs = 20000;  // boot connect attempt
    constexpr uint32_t kPortalTimeoutMs = 180000;      // AP portal lifetime (only if credentials exist)
    constexpr uint32_t kOfflineAfterMs = 30000;        // lost link -> "offline"
    constexpr uint32_t kReconnectIntervalMs = 30000;   // retry period while offline
    constexpr bool kWebPortalOnStation = true;         // settings page on the LAN IP

    // ---- Defaults for the user settings ------------------------------------
    constexpr const char *kDefaultTimezone = "Europe/Madrid";
    constexpr const char *kDefaultCity = "Madrid";
    constexpr const char *kDefaultTickers = "BTCUSDT,ETHUSDT";
    constexpr uint8_t kDefaultBrightness = 32;

    // ---- Time ---------------------------------------------------------------
    constexpr const char *kNtpServer1 = "pool.ntp.org";
    constexpr const char *kNtpServer2 = "time.google.com";
    constexpr const char *kNtpServer3 = "time.cloudflare.com";

    // ---- Internet data ------------------------------------------------------
    constexpr uint8_t kMaxTickers = 4;
    constexpr uint32_t kTickerPollMs = 15000;
    constexpr uint32_t kTickerRetryMs = 10000;
    constexpr uint32_t kWeatherPollMs = 10UL * 60UL * 1000UL; // weather is slow-moving
    constexpr uint32_t kWeatherRetryMs = 30000;
    constexpr uint32_t kHttpTimeoutMs = 8000;
    constexpr uint32_t kNetTaskStackBytes = 12288; // TLS handshakes need a deep stack

    // ---- I2C sensors --------------------------------------------------------
    constexpr uint32_t kSensorPollMs = 5000;
    constexpr uint32_t kSensorReprobeMs = 15000; // look for a hot-plugged sensor
    constexpr uint8_t kSensorMaxFailures = 3;
    constexpr float kSensorTempOffsetC = 0.0f;   // compensate enclosure self-heating

    // ---- Screens (rotation) -------------------------------------------------
    constexpr uint32_t kTimeScreenMs = 10000;
    constexpr uint32_t kDateScreenMs = 4000;
    constexpr uint32_t kWeatherScreenMs = 8000;
    constexpr uint32_t kIndoorScreenMs = 9000;
    constexpr uint32_t kStatusScreenMs = 4000;
    constexpr uint16_t kScrollSpeedMs = 40;      // ms per pixel
    constexpr uint16_t kAlertScrollSpeedMs = 30;
    constexpr uint8_t kAlertLoops = 2;
    constexpr uint16_t kFadeMs = 300;            // crossfade between static screens
    constexpr uint16_t kFadeStepMs = 20;

    // ---- Alarm (visual only: the board has no buzzer) ----------------------
    constexpr uint32_t kAlarmRingMs = 60000;
    constexpr uint16_t kAlarmBlinkMs = 250;

    // ---- Brightness ---------------------------------------------------------
    constexpr uint8_t kBrightnessLevels[] = {4, 12, 32, 64, 128};
    constexpr uint32_t kBrightnessSaveDelayMs = 3000; // coalesce NVS writes

    // ---- Buttons ------------------------------------------------------------
    constexpr uint16_t kDebounceMs = 30;
    constexpr uint16_t kLongPressMs = 800;
    constexpr uint16_t kFactoryResetHoldMs = 3000;
    constexpr uint32_t kRestartDelayMs = 6000;   // after saving settings

    // ---- Monthly celebration (ported from WuaDisplay-r1) --------------------
    constexpr int kCelebrationYear = 2025;
    constexpr int kCelebrationMonth = 11;
    constexpr int kCelebrationDay = 26;
}
