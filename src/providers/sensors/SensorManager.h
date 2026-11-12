#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "providers/sensors/Aht2xSensor.h"
#include "providers/sensors/Bme280Sensor.h"
#include "providers/sensors/ISensor.h"
#include "providers/sensors/Sht3xSensor.h"
#include "providers/sensors/Sht4xSensor.h"

// =============================================================================
//  SensorManager -- auto-detects one environmental sensor on the I2C
//  expansion port and samples it periodically, without blocking.
//
//  Supported out of the box: SHT4x, SHT3x, AHT2x, BME280. The bus is rescanned
//  every few seconds, so a sensor can be plugged in at any time; a sensor that
//  stops answering is dropped after a few failed reads.
//  Runs in the main loop (I2C transactions take well under 1 ms).
// =============================================================================
class SensorManager
{
public:
    void begin();
    void loop();

    bool hasReading() const { return _hasReading; }
    const SensorReading &reading() const { return _reading; }
    uint32_t version() const { return _version; }
    const char *sensorName() const { return _active ? _active->name() : ""; }

    // One-shot UI events (true once per occurrence).
    bool takeDetectedEvent();
    bool takeLostEvent();

private:
    enum class Phase : uint8_t
    {
        NoSensor,
        Idle,
        Converting
    };

    void probe();
    void markFailure();

    TwoWire &_wire = Wire;

    Sht4xSensor _sht4x;
    Sht3xSensor _sht3x;
    Aht2xSensor _aht2x;
    Bme280Sensor _bme280;
    // Probe order matters: SHT4x and SHT3x share 0x44/0x45 and SHT4x has the
    // stricter identification, so it goes first.
    ISensor *const _candidates[4] = {&_sht4x, &_sht3x, &_aht2x, &_bme280};

    ISensor *_active = nullptr;
    Phase _phase = Phase::NoSensor;
    uint32_t _phaseSinceMs = 0;
    uint32_t _lastProbeMs = 0;
    uint8_t _failures = 0;

    SensorReading _reading{};
    bool _hasReading = false;
    uint32_t _version = 0;
    bool _detectedEvent = false;
    bool _lostEvent = false;
};
