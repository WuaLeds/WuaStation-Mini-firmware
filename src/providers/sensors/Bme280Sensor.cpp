#include "providers/sensors/Bme280Sensor.h"

#include "providers/sensors/I2cUtil.h"

namespace
{
    constexpr uint8_t kAddresses[] = {0x76, 0x77};
}

bool Bme280Sensor::probe(TwoWire &wire)
{
    for (uint8_t address : kAddresses)
    {
        // Check the ACK first: begin() would otherwise allocate a device
        // object on every hot-plug scan.
        if (!I2cUtil::present(wire, address))
            continue;
        if (!_bme.begin(address, &wire)) // also checks the chip id (0x60)
            continue;

        // Bosch "weather monitoring" profile.
        _bme.setSampling(Adafruit_BME280::MODE_FORCED,
                         Adafruit_BME280::SAMPLING_X1,  // temperature
                         Adafruit_BME280::SAMPLING_X1,  // pressure
                         Adafruit_BME280::SAMPLING_X1,  // humidity
                         Adafruit_BME280::FILTER_OFF);
        return true;
    }
    return false;
}

bool Bme280Sensor::startMeasurement()
{
    // ~10 ms with x1 oversampling; short enough to run inline.
    return _bme.takeForcedMeasurement();
}

bool Bme280Sensor::readMeasurement(SensorReading &out)
{
    const float t = _bme.readTemperature();
    const float h = _bme.readHumidity();
    const float p = _bme.readPressure();
    if (isnan(t) || isnan(h) || isnan(p))
        return false;

    out.temperatureC = t;
    out.humidity = constrain(h, 0.0f, 100.0f);
    out.pressureHpa = p / 100.0f;
    out.hasHumidity = true;
    out.hasPressure = true;
    return true;
}
