#include "board/Board.h"

#include "board/Pins.h"

namespace Board
{
    void init()
    {
        // The ROM bootloader drives IO21 (UART0 TX) while booting; from here on
        // keep every pin wired to the USB-C CC lines high-impedance.
        for (uint8_t pin : Pins::kReservedCc)
        {
            pinMode(pin, INPUT);
        }
    }

    const char *deviceId()
    {
        static char id[5] = {0};
        if (id[0] == '\0')
        {
            // getEfuseMac() packs the 6 MAC bytes little-endian: bytes 4 and 5
            // are the device-specific tail.
            const uint64_t mac = ESP.getEfuseMac();
            snprintf(id, sizeof(id), "%02X%02X",
                     static_cast<unsigned>((mac >> 32) & 0xFF),
                     static_cast<unsigned>((mac >> 40) & 0xFF));
        }
        return id;
    }
}
