#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

// =============================================================================
//  HTTPS GET + streaming JSON parse, for the network task only (blocking).
//
//  Built on ESP-IDF's esp_http_client, so it works with whatever brings the
//  network up (ESPHome runs the WiFi through ESP-IDF directly). Servers are
//  verified against the ESP-IDF certificate bundle. The body is parsed while
//  it arrives, through an optional ArduinoJson filter, so large answers never
//  sit in RAM.
// =============================================================================
namespace HttpJson
{
    constexpr int kOk = 200;
    constexpr int kConnectError = -1; // DNS, TCP or TLS failure
    constexpr int kJsonError = -100;

    // GET `url` and deserialize the body into `doc`. Returns the HTTP status
    // (kOk on success), kConnectError or kJsonError.
    int get(const char *url, JsonDocument &doc, const JsonDocument *filter = nullptr);
    inline int get(const String &url, JsonDocument &doc, const JsonDocument *filter = nullptr)
    {
        return get(url.c_str(), doc, filter);
    }
}
