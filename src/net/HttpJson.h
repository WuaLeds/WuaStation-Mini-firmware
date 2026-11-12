#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

// =============================================================================
//  HTTPS GET + streaming JSON parse, for the network task only (blocking).
//
//  TLS is used without certificate validation (setInsecure): the data sources
//  are public, read-only APIs and the C3 has no room for a CA bundle update
//  workflow. Do not reuse this helper for anything that sends secrets.
// =============================================================================
namespace HttpJson
{
    constexpr int kJsonError = -100;

    // GET `url` and deserialize the body into `doc` (optionally through an
    // ArduinoJson filter). Returns the HTTP status (200 on success), a
    // negative HTTPClient error, or kJsonError.
    int get(const String &url, JsonDocument &doc, const JsonDocument *filter = nullptr);
}
