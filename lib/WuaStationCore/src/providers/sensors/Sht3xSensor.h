#pragma once

#include "providers/sensors/ISensor.h"

// Sensirion SHT30 / SHT31 / SHT35 -- temperature + humidity.
class Sht3xSensor : public ISensor
{
public:
    const char *name() const override { return "SHT3x"; }
    bool probe(TwoWire &wire) override;
    bool startMeasurement() override;
    uint16_t conversionTimeMs() const override { return 16; } // high repeatability: 15.5 ms max
    bool readMeasurement(SensorReading &out) override;

private:
    TwoWire *_wire = nullptr;
    uint8_t _address = 0;
};
