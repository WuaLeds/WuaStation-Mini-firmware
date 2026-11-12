#include "providers/sensors/Sht4xSensor.h"

#include "providers/sensors/I2cUtil.h"

namespace
{
    constexpr uint8_t kAddresses[] = {0x44, 0x45, 0x46};
    constexpr uint8_t kCmdSoftReset = 0x94;
    constexpr uint8_t kCmdSerial = 0x89;
    constexpr uint8_t kCmdMeasureHigh = 0xFD;
}

bool Sht4xSensor::probe(TwoWire &wire)
{
    for (uint8_t address : kAddresses)
    {
        if (!I2cUtil::present(wire, address))
            continue;

        // SHT4x speaks 1-byte commands: soft reset, then read the serial
        // number (two CRC-protected words) to confirm the chip family.
        I2cUtil::write(wire, address, &kCmdSoftReset, 1);
        delay(2);
        if (!I2cUtil::write(wire, address, &kCmdSerial, 1))
            continue;
        delay(2);
        uint8_t buf[6];
        if (!I2cUtil::read(wire, address, buf, sizeof(buf)))
            continue;
        if (!I2cUtil::wordValid(buf) || !I2cUtil::wordValid(buf + 3))
            continue;

        _wire = &wire;
        _address = address;
        return true;
    }
    return false;
}

bool Sht4xSensor::startMeasurement()
{
    return I2cUtil::write(*_wire, _address, &kCmdMeasureHigh, 1);
}

bool Sht4xSensor::readMeasurement(SensorReading &out)
{
    uint8_t buf[6];
    if (!I2cUtil::read(*_wire, _address, buf, sizeof(buf)))
        return false;
    if (!I2cUtil::wordValid(buf) || !I2cUtil::wordValid(buf + 3))
        return false;

    const uint16_t rawT = static_cast<uint16_t>((buf[0] << 8) | buf[1]);
    const uint16_t rawH = static_cast<uint16_t>((buf[3] << 8) | buf[4]);
    out.temperatureC = -45.0f + 175.0f * rawT / 65535.0f;
    out.humidity = constrain(-6.0f + 125.0f * rawH / 65535.0f, 0.0f, 100.0f);
    out.hasHumidity = true;
    out.hasPressure = false;
    return true;
}
