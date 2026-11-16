#pragma once

#include "config/Strings.h"
#include "net/WifiService.h"
#include "states/ScrollTextStateBase.h"

// Interrupt screen while the WiFi configuration access point is open:
// keeps scrolling the AP name and password until the portal closes.
class PortalStateProvider : public ScrollTextStateBase
{
public:
    static constexpr uint8_t kPriority = 2; // above a pinned timer

    PortalStateProvider(Compositor &compositor, WifiService &wifi)
        : ScrollTextStateBase(compositor, "portal", 0, 0 /* endless */, kPriority), _wifi(wifi) {}

    bool canBeShown() const override { return _wifi.portalActive(); }

    bool onButton(ButtonId id, ButtonEvent event) override
    {
        // UP long-press toggles the portal off again.
        if (id == ButtonId::Up && event == ButtonEvent::LongPress)
        {
            _wifi.cancelPortal();
            return true;
        }
        return false;
    }

protected:
    bool nextPass(String &text, Rgb &color) override
    {
        char buf[128];
        const char *pass = _wifi.apPassword();
        if (pass && pass[0])
            snprintf(buf, sizeof(buf), Strings::kPortalFmt, _wifi.apName(), pass);
        else
            snprintf(buf, sizeof(buf), Strings::kPortalOpenFmt, _wifi.apName());
        text = buf;
        color = Theme::kPortal;
        return true;
    }

private:
    WifiService &_wifi;
};
