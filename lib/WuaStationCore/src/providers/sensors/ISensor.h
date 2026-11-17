#pragma once

#include <Arduino.h>
#include <Wire.h>

struct SensorReading
{
    float temperatureC;
    float humidity;    // %RH, valid if hasHumidity
    float pressureHpa; // valid if hasPressure
    bool hasHumidity;
    bool hasPressure;
    uint32_t timestampMs;
};

// =============================================================================
//  ISensor -- one environmental sensor on the I2C expansion port.
//
//  Measurements are split in start + read so slow conversions (e.g. 80 ms on
//  the AHT20) never block the display loop: SensorManager calls
//  startMeasurement(), waits conversionTimeMs() and then readMeasurement().
//  To add a sensor: implement this interface and register it in SensorManager.
// =============================================================================
class ISensor
{
public:
    virtual ~ISensor() = default;

    virtual const char *name() const = 0;

    // Detect the chip on the bus and initialize it. True if found.
    virtual bool probe(TwoWire &wire) = 0;

    virtual bool startMeasurement() = 0;
    virtual uint16_t conversionTimeMs() const = 0;
    virtual bool readMeasurement(SensorReading &out) = 0;
};
