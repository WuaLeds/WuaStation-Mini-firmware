#pragma once

#include "config/Strings.h"
#include "states/ScrollTextStateBase.h"

// Boot greeting: scrolls the device name once while WiFi connects.
class SplashStateProvider : public ScrollTextStateBase
{
public:
    explicit SplashStateProvider(Compositor &compositor)
        : ScrollTextStateBase(compositor, "splash", 0, 1) {}

    bool canBeShown() const override { return !_done; }

protected:
    bool nextPass(String &text, Rgb &color) override
    {
        text = Strings::kSplash;
        color = Theme::kSplash;
        return true;
    }

    void onFinished() override { _done = true; }

private:
    bool _done = false;
};
