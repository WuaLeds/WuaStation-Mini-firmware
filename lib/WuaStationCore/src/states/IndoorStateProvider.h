#pragma once

#include "providers/sensors/SensorManager.h"
#include "states/StaticStateBase.h"

// Indoor climate from the I2C sensor: temperature (house icon), humidity
// (drop icon) and, when the sensor has it, pressure in hPa.
class IndoorStateProvider : public StaticStateBase
{
public:
    IndoorStateProvider(Compositor &compositor, const SensorManager &sensors);

    bool canBeShown() const override { return _sensors.hasReading(); }

protected:
    void onShow() override;
    void tick() override;
    void render(Adafruit_GFX &canvas) override;

private:
    uint8_t phaseCount() const;
    uint8_t currentPhase() const;

    const SensorManager &_sensors;
    SensorReading _reading{};
    uint32_t _version = 0;
    uint8_t _phase = 0;
};
