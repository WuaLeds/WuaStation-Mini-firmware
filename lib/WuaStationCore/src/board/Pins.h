#pragma once

#include <Arduino.h>

// =============================================================================
//  WuaStation-mini pinout -- ESP32-C3-WROOM-02-N4
// -----------------------------------------------------------------------------
//  Source of truth: PCB/WuaStation-Mini/WuaStation-Mini.kicad_sch
//
//    Net      GPIO   Notes
//    DIN      IO5    LMX1p data in (126 x SK6805-EC15)
//    UP       IO9    SW2 to GND. BOOT strap: held at reset -> download mode
//    DOWN     IO4    SW3 to GND
//    TOUCH    IO7    USB-C SBU1/SBU2, 2k pull-down (R8) -> active HIGH
//    SDA      IO3    J1 expansion connector (no pull-ups on board)
//    SCL      IO6    J1 expansion connector (no pull-ups on board)
//    MCC1     IO0 + IO20   USB-C CC1 through JP1 (bridged)
//    MCC2     IO1 + IO21   USB-C CC2 through JP2 (bridged)
//    D- / D+  IO18 / IO19  native USB (Serial = USB-CDC)
//    RST      EN     SW1
// =============================================================================

#ifndef LMX1P_LED_PIN
#error "LMX1P_LED_PIN must be defined in platformio.ini (LMX1p DIN = GPIO5)"
#endif

namespace Pins
{
    // LMX1p panoramic matrix. FastLED needs the data pin as a compile-time
    // constant, so the value lives in platformio.ini (-D LMX1P_LED_PIN=5) where
    // the WuaDisplay-LMX library sees it as well.
    constexpr uint8_t kLedData = LMX1P_LED_PIN;

    // Push buttons to GND, no external pull-up -> INPUT_PULLUP.
    constexpr uint8_t kButtonUp = 9;
    constexpr uint8_t kButtonDown = 4;

    // External touch input (e.g. a TTP223 module on the USB-C SBU pins).
    // Idle LOW thanks to the on-board 2k pull-down, active HIGH.
    constexpr uint8_t kTouch = 7;

    // I2C expansion port (SDA, SCL, GND, 3V3). The sensor breakout must
    // provide the pull-ups; the internal ones are enabled as a fallback only.
    constexpr uint8_t kI2cSda = 3;
    constexpr uint8_t kI2cScl = 6;
    constexpr uint32_t kI2cFrequency = 100000;

    // Tied to the USB-C CC lines through solder jumpers JP1/JP2 (bridged by
    // default). They are parked as high-impedance inputs at boot so the 5.1k
    // Rd resistors keep the USB-C power negotiation working. IO20/IO21 are
    // UART0: never use Serial0 on this board.
    constexpr uint8_t kReservedCc[] = {0, 20, 1, 21};
}
