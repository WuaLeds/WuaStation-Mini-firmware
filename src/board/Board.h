#pragma once

#include <Arduino.h>

// Low-level board bring-up and identity.
namespace Board
{
    // Park reserved pins in a safe state. Call first thing in setup().
    void init();

    // Short, stable device id derived from the MAC (e.g. "1A2B").
    const char *deviceId();
}
