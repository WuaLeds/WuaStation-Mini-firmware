#include "providers/sensors/Sht3xSensor.h"

#include "providers/sensors/I2cUtil.h"

namespace
{
    constexpr uint8_t kAddresses[] = {0x44, 0x45};
    constexpr uint16_t kCmdSoftReset = 0x30A2;
    constexpr uint16_t kCmdReadStatus = 0xF32D;
    constexpr uint16_t kCmdSingleShotHigh = 0x2400; // no clock stretching
}

bool Sht3xSensor::probe(TwoWire &wire)
{
    for (uint8_t address : kAddresses)
    {
        if (!I2cUtil::present(wire, address))
            continue;

        I2cUtil::writeCommand16(wire, address, kCmdSoftReset);
        delay(2);
        // The status register comes back as one CRC-protected word.
        if (!I2cUtil::writeCommand16(wire, address, kCmdReadStatus))
            continue;
        uint8_t buf[3];
        if (!I2cUtil::read(wire, address, buf, sizeof(buf)) || !I2cUtil::wordValid(buf))
            continue;

        _wire = &wire;
        _address = address;
        return true;
    }
    return false;
}

bool Sht3xSensor::startMeasurement()
{
    return I2cUtil::writeCommand16(*_wire, _address, kCmdSingleShotHigh);
}

bool Sht3xSensor::readMeasurement(SensorReading &out)
{
    uint8_t buf[6];
    if (!I2cUtil::read(*_wire, _address, buf, sizeof(buf)))
        return false;
    if (!I2cUtil::wordValid(buf) || !I2cUtil::wordValid(buf + 3))
        return false;

    const uint16_t rawT = static_cast<uint16_t>((buf[0] << 8) | buf[1]);
    const uint16_t rawH = static_cast<uint16_t>((buf[3] << 8) | buf[4]);
    out.temperatureC = -45.0f + 175.0f * rawT / 65535.0f;
    out.humidity = constrain(100.0f * rawH / 65535.0f, 0.0f, 100.0f);
    out.hasHumidity = true;
    out.hasPressure = false;
    return true;
}
