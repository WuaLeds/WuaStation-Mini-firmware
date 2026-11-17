#include "providers/sensors/Aht2xSensor.h"

#include "providers/sensors/I2cUtil.h"

namespace
{
    constexpr uint8_t kAddress = 0x38;
    constexpr uint8_t kStatusBusy = 0x80;
    constexpr uint8_t kStatusCalibrated = 0x08;
    const uint8_t kCmdInit[] = {0xBE, 0x08, 0x00};
    const uint8_t kCmdMeasure[] = {0xAC, 0x33, 0x00};
    constexpr uint8_t kCmdStatus = 0x71;
}

bool Aht2xSensor::readStatus(uint8_t &status)
{
    return I2cUtil::write(*_wire, kAddress, &kCmdStatus, 1) && I2cUtil::read(*_wire, kAddress, &status, 1);
}

bool Aht2xSensor::probe(TwoWire &wire)
{
    if (!I2cUtil::present(wire, kAddress))
        return false;
    _wire = &wire;

    uint8_t status = 0;
    if (!readStatus(status))
        return false;
    if (!(status & kStatusCalibrated))
    {
        // Load the calibration coefficients.
        I2cUtil::write(wire, kAddress, kCmdInit, sizeof(kCmdInit));
        delay(10);
        if (!readStatus(status) || !(status & kStatusCalibrated))
            return false;
    }
    return true;
}

bool Aht2xSensor::startMeasurement()
{
    return I2cUtil::write(*_wire, kAddress, kCmdMeasure, sizeof(kCmdMeasure));
}

bool Aht2xSensor::readMeasurement(SensorReading &out)
{
    // status, 5 data bytes (20-bit RH + 20-bit T), CRC
    uint8_t buf[7];
    if (!I2cUtil::read(*_wire, kAddress, buf, sizeof(buf)))
        return false;
    if (buf[0] & kStatusBusy)
        return false;
    if (I2cUtil::crc8(buf, 6) != buf[6])
        return false;

    const uint32_t rawH = (static_cast<uint32_t>(buf[1]) << 12) | (static_cast<uint32_t>(buf[2]) << 4) | (buf[3] >> 4);
    const uint32_t rawT = (static_cast<uint32_t>(buf[3] & 0x0F) << 16) | (static_cast<uint32_t>(buf[4]) << 8) | buf[5];
    out.humidity = constrain(rawH * 100.0f / 1048576.0f, 0.0f, 100.0f);
    out.temperatureC = rawT * 200.0f / 1048576.0f - 50.0f;
    out.hasHumidity = true;
    out.hasPressure = false;
    return true;
}
