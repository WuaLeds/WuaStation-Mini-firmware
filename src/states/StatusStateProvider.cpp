#include "states/StatusStateProvider.h"

#include "config/Config.h"
#include "config/Theme.h"
#include "display/Icons.h"

namespace
{
    constexpr uint16_t kFrameMs = 350;
}

StatusStateProvider::StatusStateProvider(Compositor &compositor, WifiService &wifi, const TimeService &time)
    : StaticStateBase(compositor, "status", Config::kStatusScreenMs), _wifi(wifi), _time(time)
{
}

bool StatusStateProvider::canBeShown() const
{
    return !_wifi.portalActive() && (!_wifi.isConnected() || !_time.isSynced());
}

void StatusStateProvider::tick()
{
    const uint8_t frame = static_cast<uint8_t>((millis() / kFrameMs) % 4);
    if (frame != _frame)
    {
        _frame = frame;
        invalidate();
    }
}

void StatusStateProvider::render(Adafruit_GFX &canvas)
{
    const bool connected = _wifi.isConnected();
    const bool offline = _wifi.state() == WifiService::State::Offline;

    const Rgb iconColor = offline ? Theme::kWifiOffline : (connected ? Theme::kWifiSyncing : Theme::kWifiConnecting);
    Icons::drawTinted(canvas, Icons::kWifi, 0, 0, iconColor.to565());

    if (offline)
    {
        // Red cross, 5x5, centered in the right-hand area.
        const uint16_t c = Theme::kStatusError.to565();
        canvas.drawLine(10, 1, 14, 5, c);
        canvas.drawLine(14, 1, 10, 5, c);
        return;
    }

    // "..." progress, 2x2 dots appearing one by one.
    const uint16_t c = Theme::kStatusDots.to565();
    const uint8_t dots = _frame == 0xFF ? 0 : _frame; // 0..3
    for (uint8_t i = 0; i < dots; ++i)
        canvas.fillRect(9 + i * 3, 4, 2, 2, c);
}
