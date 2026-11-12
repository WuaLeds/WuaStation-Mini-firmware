#pragma once

#include "providers/sensors/ISensor.h"

// Aosong AHT20 / AHT21 / AHT25 -- temperature + humidity.
class Aht2xSensor : public ISensor
{
public:
    const char *name() const override { return "AHT2x"; }
    bool probe(TwoWire &wire) override;
    bool startMeasurement() override;
    uint16_t conversionTimeMs() const override { return 85; } // 80 ms typical
    bool readMeasurement(SensorReading &out) override;

private:
    bool readStatus(uint8_t &status);

    TwoWire *_wire = nullptr;
};
