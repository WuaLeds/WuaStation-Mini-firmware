#pragma once

// =============================================================================
//  User-facing texts shown on the matrix. Kept in one place so they can be
//  translated. UTF-8 is fine: TextUtil converts it to the display charset
//  (accents, n-tilde, inverted marks, degree sign...).
// =============================================================================
namespace Strings
{
    constexpr const char *kSplash = "WuaStation";

    // printf-style: AP name, AP password
    constexpr const char *kPortalFmt = "WiFi setup: join %s  pass %s  open 192.168.4.1";
    constexpr const char *kPortalOpenFmt = "WiFi setup: join %s  open 192.168.4.1";

    // printf-style: IP, hostname, firmware version, Home Assistant note
    constexpr const char *kInfoOnlineFmt = "IP %s  %s.local  v%s%s";
    constexpr const char *kInfoHomeAssistant = "  Home Assistant connected";
    constexpr const char *kInfoOffline = "Offline - the WiFi setup opens by itself if the network is gone";
    // printf-style: network name, IP, hostname
    constexpr const char *kWifiInfoFmt = "WiFi %s  %s  %s.local";

    constexpr const char *kSettingsSaved = "Saved, restarting";
    constexpr const char *kSensorFoundFmt = "Sensor %s";
    constexpr const char *kSensorLost = "Sensor lost";
    constexpr const char *kTimezoneUnknownFmt = "Unknown timezone %s, using UTC";
    constexpr const char *kTickerInvalidFmt = "Ticker %s not found";

    constexpr const char *kFactoryResetPrompt = "RST";
    constexpr const char *kFactoryResetDone = "OK";

    // printf-style: YYYY, MM, DD
    constexpr const char *kCalendarFmt = "Today is %04d-%02d-%02d";
    // printf-style: total months, years, months
    constexpr const char *kCelebrationFmt =
        "Celebration day!! It's been %d months since the WuaDisplay state event firmware!! (%d years %d months)";
}
