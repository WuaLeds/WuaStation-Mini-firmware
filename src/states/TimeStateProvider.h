#pragma once

#include "net/TimeService.h"
#include "states/StaticStateBase.h"

// Clock: "HH:MM" in TinyFont (fits the 18 px width) with a blinking colon.
class TimeStateProvider : public StaticStateBase
{
public:
    TimeStateProvider(Compositor &compositor, const TimeService &time);

    bool canBeShown() const override { return _time.isSynced(); }

protected:
    void onShow() override { _shownKey = -1; }
    void tick() override;
    void render(Adafruit_GFX &canvas) override;

private:
    const TimeService &_time;
    int _shownKey = -1; // minute of day * 2 + colon phase
};
