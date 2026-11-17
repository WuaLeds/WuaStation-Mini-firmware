#pragma once

#include "providers/sensors/ISensor.h"

// Sensirion SHT40 / SHT41 / SHT43 / SHT45 -- temperature + humidity.
class Sht4xSensor : public ISensor
{
public:
    const char *name() const override { return "SHT4x"; }
    bool probe(TwoWire &wire) override;
    bool startMeasurement() override;
    uint16_t conversionTimeMs() const override { return 10; } // high precision: 8.3 ms max
    bool readMeasurement(SensorReading &out) override;

private:
    TwoWire *_wire = nullptr;
    uint8_t _address = 0;
};
