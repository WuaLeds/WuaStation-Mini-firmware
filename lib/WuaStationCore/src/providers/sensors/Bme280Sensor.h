#pragma once

#include <Adafruit_BME280.h>
#include "providers/sensors/ISensor.h"

// Bosch BME280 -- temperature + humidity + pressure (Adafruit driver).
// Used in forced mode (one conversion per poll) to avoid self-heating.
class Bme280Sensor : public ISensor
{
public:
    const char *name() const override { return "BME280"; }
    bool probe(TwoWire &wire) override;
    bool startMeasurement() override;
    uint16_t conversionTimeMs() const override { return 0; } // done in startMeasurement()
    bool readMeasurement(SensorReading &out) override;

private:
    Adafruit_BME280 _bme;
};
