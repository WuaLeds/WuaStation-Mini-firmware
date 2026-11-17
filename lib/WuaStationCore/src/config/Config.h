#pragma once

#include <Arduino.h>

// =============================================================================
//  Firmware configuration: compile-time defaults and tuning constants.
//  Everything the user can change at runtime (timezone, city, tickers...) is
//  only the *default* here; the live value is stored in NVS (core/Settings)
//  and edited from the settings page or Home Assistant.
// =============================================================================

#define WUA_FIRMWARE_VERSION "0.4"

namespace Config
{
    // ---- Identity / WiFi (the WiFi itself is run by ESPHome) ---------------
    constexpr const char *kDeviceName = "WuaStation";      // setup AP: "WuaStation-XXXX"
    constexpr const char *kPortalPassword = "WuaLeds2025"; // setup AP password (>= 8 chars, "" = open)
    constexpr uint32_t kOfflineAfterMs = 30000;            // link lost this long -> "offline" icon

    // ---- Defaults for the user settings ------------------------------------
    // "auto": the clock follows the weather city. Otherwise a zone name
    // ("Buenos Aires", "Europe/Madrid") or a POSIX TZ string.
    constexpr const char *kDefaultTimezone = "auto";
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
    constexpr float kSensorTempOffsetC = 0.0f; // default; editable from the settings page
    constexpr float kSensorTempOffsetMaxC = 10.0f;

    // ---- Screens (rotation) -------------------------------------------------
    // Default rotation, editable from the settings page ("Screens"): ids in
    // display order, optional ":seconds". See config/ScreenConfig for the ids.
    constexpr const char *kDefaultScreens =
        "time:10,date:4,weather:8,forecast:10,indoor:9,ticker,message,celebration,timer:4";

    // Default durations (used when the screens setting gives no seconds)
    constexpr uint32_t kTimeScreenMs = 10000;
    constexpr uint32_t kDateScreenMs = 4000;
    constexpr uint32_t kWeatherScreenMs = 8000;
    constexpr uint32_t kForecastScreenMs = 10000; // shared by its pages
    constexpr uint32_t kIndoorScreenMs = 9000;
    constexpr uint32_t kTimerScreenMs = 4000; // timer screen while idle
    constexpr uint32_t kStatusScreenMs = 4000;
    constexpr uint32_t kMaxScreenSeconds = 600;

    constexpr uint16_t kScrollSpeedMs = 40; // ms per pixel
    constexpr uint16_t kAlertScrollSpeedMs = 30;
    constexpr uint8_t kAlertLoops = 2;
    constexpr uint16_t kFadeMs = 300; // crossfade between static screens
    constexpr uint16_t kFadeStepMs = 20;
    constexpr uint32_t kPanHoldMs = 700; // a page wider than 18 px pans slowly

    // ---- Ticker: fixed pages per symbol, name -> price -> 24 h change ------
    // "ticker:N" in the screens setting = N seconds per symbol, split in the
    // same proportions.
    constexpr uint32_t kTickerNameMs = 1500;
    constexpr uint32_t kTickerPriceMs = 3000;
    constexpr uint32_t kTickerChangeMs = 2000;
    constexpr uint32_t kTickerSymbolMs = kTickerNameMs + kTickerPriceMs + kTickerChangeMs;
    constexpr float kTickerPriceMaxError = 0.01f; // abbreviation ("97.2K") may round <= 1 %

    // ---- Air quality (default; editable from the settings page) ------------
    constexpr bool kAirQualityEuropean = false; // false: US AQI (0-500), true: European AQI

    // ---- Timer / pomodoro ---------------------------------------------------
    constexpr uint8_t kTimerPresetsMin[] = {1, 3, 5, 10, 15, 20, 30, 45, 60, 90}; // then pomodoro
    constexpr uint8_t kTimerDefaultPreset = 2;                                    // index in kTimerPresetsMin (5 min)
    constexpr uint8_t kPomodoroWorkMin = 25;
    constexpr uint8_t kPomodoroShortBreakMin = 5;
    constexpr uint8_t kPomodoroLongBreakMin = 15;
    constexpr uint8_t kPomodoroCycles = 4;     // work blocks before a long break
    constexpr uint32_t kTimerRingMs = 30000;   // countdown finished: flash this long
    constexpr uint32_t kPomodoroRingMs = 8000; // pomodoro phase change: flash, then go on
    constexpr uint32_t kTimerHoldMs = 6000;    // idle timer screen stays after a button

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
    constexpr uint32_t kRestartDelayMs = 4000;   // after a change that needs one (city, tickers)
    constexpr uint32_t kRestartNowDelayMs = 800; // restart button: let the reply go out

    // ---- Monthly celebration (ported from WuaDisplay-r1) --------------------
    constexpr int kCelebrationYear = 2025;
    constexpr int kCelebrationMonth = 11;
    constexpr int kCelebrationDay = 26;
}
