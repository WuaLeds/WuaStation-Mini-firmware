#pragma once

#include "app/Host.h"
#include "config/Strings.h"
#include "states/ScrollTextStateBase.h"

// =============================================================================
//  Interrupt screen while the WiFi setup access point is open (no network
//  saved, or the saved one out of reach): keeps scrolling the AP name and
//  password until the station joins a network.
// =============================================================================
class PortalStateProvider : public ScrollTextStateBase
{
public:
    static constexpr uint8_t kPriority = 2; // above a pinned timer

    PortalStateProvider(Compositor &compositor, const Host &host)
        : ScrollTextStateBase(compositor, "portal", 0, 0 /* endless */, kPriority), _host(host)
    {
    }

    bool canBeShown() const override { return _host.setupActive(); }

protected:
    bool nextPass(String &text, Rgb &color) override
    {
        char buf[128];
        snprintf(buf, sizeof(buf), Strings::kPortalFmt, _host.apName());
        text = buf;
        color = Theme::kPortal;
        return true;
    }

private:
    const Host &_host;
};
