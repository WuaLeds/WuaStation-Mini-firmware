#include "providers/sensors/SensorManager.h"

#include "board/Pins.h"
#include "config/Config.h"
#include "core/Logger.h"

void SensorManager::begin()
{
    // The breakout should carry its own pull-ups; the weak internal ones the
    // I2C driver enables only help with short wires.
    _wire.begin(Pins::kI2cSda, Pins::kI2cScl, Pins::kI2cFrequency);
    _wire.setTimeOut(20);
    probe();
}

void SensorManager::probe()
{
    _lastProbeMs = millis();
    for (ISensor *sensor : _candidates)
    {
        if (sensor->probe(_wire))
        {
            _active = sensor;
            _phase = Phase::Idle;
            _phaseSinceMs = 0; // measure right away
            _failures = 0;
            _detectedEvent = true;
            LOG_INFO("Sensor detected: %s", sensor->name());
            return;
        }
    }
}

void SensorManager::markFailure()
{
    _phase = Phase::Idle;
    _phaseSinceMs = millis();
    if (++_failures < Config::kSensorMaxFailures)
        return;

    LOG_WARN("Sensor %s lost", _active->name());
    _active = nullptr;
    _phase = Phase::NoSensor;
    _hasReading = false;
    _lostEvent = true;
    ++_version;
}

void SensorManager::loop()
{
    const uint32_t now = millis();

    switch (_phase)
    {
    case Phase::NoSensor:
        if (now - _lastProbeMs >= Config::kSensorReprobeMs)
            probe();
        break;

    case Phase::Idle:
        if (_phaseSinceMs != 0 && now - _phaseSinceMs < Config::kSensorPollMs)
            break;
        if (!_active->startMeasurement())
        {
            markFailure();
            break;
        }
        _phase = Phase::Converting;
        _phaseSinceMs = now;
        break;

    case Phase::Converting:
    {
        if (now - _phaseSinceMs < _active->conversionTimeMs())
            break;
        SensorReading r{};
        if (!_active->readMeasurement(r))
        {
            markFailure();
            break;
        }
        r.temperatureC += Config::kSensorTempOffsetC;
        r.timestampMs = now;
        _reading = r;
        _hasReading = true;
        _failures = 0;
        ++_version;
        _phase = Phase::Idle;
        _phaseSinceMs = now;
        LOG_DEBUG("Sensor %s: %.2f C, %.1f %%, %.1f hPa", _active->name(), r.temperatureC,
                  r.humidity, r.pressureHpa);
        break;
    }
    }
}

bool SensorManager::takeDetectedEvent()
{
    const bool e = _detectedEvent;
    _detectedEvent = false;
    return e;
}

bool SensorManager::takeLostEvent()
{
    const bool e = _lostEvent;
    _lostEvent = false;
    return e;
}
