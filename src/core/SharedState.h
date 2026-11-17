#pragma once

#include <Arduino.h>
#include <type_traits>
#include "freertos/FreeRTOS.h"

// =============================================================================
//  SharedState<T> -- a value written by the network task and read by the main
//  loop. Each write bumps a version so readers can cheaply detect new data.
//
//  T must be trivially copyable (plain structs with char arrays, no String):
//  the copy happens inside a short critical section.
// =============================================================================
template <class T> class SharedState
{
    static_assert(std::is_trivially_copyable<T>::value,
                  "SharedState<T> requires a trivially copyable T (no String members)");

public:
    void write(const T &value)
    {
        portENTER_CRITICAL(&_mux);
        _value = value;
        ++_version;
        portEXIT_CRITICAL(&_mux);
    }

    // Copies the current value into `out` and returns its version (0 = never written).
    uint32_t read(T &out) const
    {
        portENTER_CRITICAL(&_mux);
        out = _value;
        const uint32_t v = _version;
        portEXIT_CRITICAL(&_mux);
        return v;
    }

    uint32_t version() const
    {
        portENTER_CRITICAL(&_mux);
        const uint32_t v = _version;
        portEXIT_CRITICAL(&_mux);
        return v;
    }

private:
    mutable portMUX_TYPE _mux = portMUX_INITIALIZER_UNLOCKED;
    T _value{};
    uint32_t _version = 0;
};
