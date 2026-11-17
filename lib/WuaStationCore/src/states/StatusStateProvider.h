#pragma once

#include "net/TimeService.h"
#include "app/Host.h"
#include "states/StaticStateBase.h"

// Connection status while there is no time to show: WiFi icon plus
// animated dots (connecting / syncing the clock) or a red cross (offline).
class StatusStateProvider : public StaticStateBase
{
public:
    StatusStateProvider(Compositor &compositor, const Host &host, const TimeService &time);

    bool canBeShown() const override;

protected:
    void onShow() override { _frame = 0xFF; }
    void tick() override;
    void render(Adafruit_GFX &canvas) override;

private:
    const Host &_host;
    const TimeService &_time;
    uint8_t _frame = 0;
};
