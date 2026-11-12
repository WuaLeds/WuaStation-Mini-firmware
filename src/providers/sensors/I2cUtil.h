#pragma once

#include <Arduino.h>
#include <Wire.h>

// Small helpers shared by the I2C sensor drivers.
namespace I2cUtil
{
    // True if a device ACKs its address.
    inline bool present(TwoWire &wire, uint8_t address)
    {
        wire.beginTransmission(address);
        return wire.endTransmission() == 0;
    }

    inline bool write(TwoWire &wire, uint8_t address, const uint8_t *data, size_t len)
    {
        wire.beginTransmission(address);
        wire.write(data, len);
        return wire.endTransmission() == 0;
    }

    // 16-bit big-endian command (Sensirion style).
    inline bool writeCommand16(TwoWire &wire, uint8_t address, uint16_t command)
    {
        const uint8_t buf[2] = {static_cast<uint8_t>(command >> 8), static_cast<uint8_t>(command & 0xFF)};
        return write(wire, address, buf, sizeof(buf));
    }

    inline bool read(TwoWire &wire, uint8_t address, uint8_t *buf, size_t len)
    {
        // (uint8_t, uint8_t) overload: unambiguous across Arduino cores.
        if (static_cast<size_t>(wire.requestFrom(address, static_cast<uint8_t>(len))) != len)
            return false;
        for (size_t i = 0; i < len; ++i)
            buf[i] = static_cast<uint8_t>(wire.read());
        return true;
    }

    // CRC-8, polynomial 0x31, init 0xFF (Sensirion SHTxx, Aosong AHT2x).
    inline uint8_t crc8(const uint8_t *data, size_t len)
    {
        uint8_t crc = 0xFF;
        for (size_t i = 0; i < len; ++i)
        {
            crc ^= data[i];
            for (uint8_t bit = 0; bit < 8; ++bit)
                crc = (crc & 0x80) ? static_cast<uint8_t>((crc << 1) ^ 0x31) : static_cast<uint8_t>(crc << 1);
        }
        return crc;
    }

    // Check a [msb, lsb, crc] Sensirion word.
    inline bool wordValid(const uint8_t *word)
    {
        return crc8(word, 2) == word[2];
    }
}
