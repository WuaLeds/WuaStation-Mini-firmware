#pragma once

#include "net/TimeService.h"
#include "net/WifiService.h"
#include "states/StaticStateBase.h"

// Connection status while there is no time to show: WiFi icon plus
// animated dots (connecting / syncing the clock) or a red cross (offline).
class StatusStateProvider : public StaticStateBase
{
public:
    StatusStateProvider(Compositor &compositor, WifiService &wifi, const TimeService &time);

    bool canBeShown() const override;

protected:
    void onShow() override { _frame = 0xFF; }
    void tick() override;
    void render(Adafruit_GFX &canvas) override;

private:
    WifiService &_wifi;
    const TimeService &_time;
    uint8_t _frame = 0;
};
