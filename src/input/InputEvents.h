#pragma once

#include <Arduino.h>

// Physical inputs of the WuaStation-mini.
enum class ButtonId : uint8_t
{
    Up,
    Down,
    Touch
};

enum class ButtonEvent : uint8_t
{
    None,
    Click,
    LongPress
};
