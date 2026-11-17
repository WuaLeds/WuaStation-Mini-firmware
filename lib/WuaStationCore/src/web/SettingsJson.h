#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#include "app/Host.h"
#include "core/Settings.h"
#include "net/TimeService.h"
#include "providers/sensors/SensorManager.h"

// =============================================================================
//  JSON of the settings page API (web/settings.html), independent of the web
//  server that carries it (the ESPHome one, in components/wuastation).
//
//    status   : firmware, network, Home Assistant, clock, sensor, uptime
//    settings : every setting + the screen list and the timezone table
//    read()   : a POSTed settings object -> Settings (fields present only)
// =============================================================================
namespace SettingsJson
{
    void writeStatus(JsonObject out, const Host &host, const TimeService &time, const SensorManager &sensors);

    // The document references the strings of `settings` (no copy): keep it
    // alive until the document has been serialized.
    void writeSettings(JsonObject out, const Settings &settings);

    // Copy the fields present in `in` into `settings` (strings are cut on a
    // UTF-8 character boundary). False if `in` is not an object.
    bool read(JsonVariantConst in, Settings &settings);
}
