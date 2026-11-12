#pragma once

#include "net/TimeService.h"
#include "states/StaticStateBase.h"

// Date: "DD.MM" in TinyFont.
class DateStateProvider : public StaticStateBase
{
public:
    DateStateProvider(Compositor &compositor, const TimeService &time);

    bool canBeShown() const override { return _time.isSynced(); }

protected:
    void onShow() override { _shownDay = -1; }
    void tick() override;
    void render(Adafruit_GFX &canvas) override;

private:
    const TimeService &_time;
    int _shownDay = -1;
};
