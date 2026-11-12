#pragma once

#include "core/Settings.h"
#include "states/ScrollTextStateBase.h"

// The user's custom message (set from the portal), scrolled once per rotation.
class MessageStateProvider : public ScrollTextStateBase
{
public:
    MessageStateProvider(Compositor &compositor, const SettingsStore &settings)
        : ScrollTextStateBase(compositor, "message", 0, 1), _settings(settings) {}

    bool canBeShown() const override { return _settings.get().message[0] != '\0'; }

protected:
    bool nextPass(String &text, Rgb &color) override
    {
        text = _settings.get().message;
        color = Theme::kMessage;
        return true;
    }

private:
    const SettingsStore &_settings;
};
