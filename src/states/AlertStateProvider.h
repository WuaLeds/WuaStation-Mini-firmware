#pragma once

#include "states/ScrollTextStateBase.h"

// =============================================================================
//  Interrupt screen for short notifications ("Sensor SHT4x", "Saved"...).
//  Messages are queued; each one scrolls `loops` times. Any click dismisses
//  the current message.
// =============================================================================
class AlertStateProvider : public ScrollTextStateBase
{
public:
    static constexpr uint8_t kPriority = 3;
    static constexpr uint8_t kQueueSize = 4;

    explicit AlertStateProvider(Compositor &compositor)
        : ScrollTextStateBase(compositor, "alert", 0, 1, kPriority, Config::kAlertScrollSpeedMs) {}

    // Queue a message (UTF-8). When the queue is full the oldest is dropped.
    void push(const char *message, Rgb color = Theme::kAlert, uint8_t loops = Config::kAlertLoops);

    bool canBeShown() const override { return _active || _count > 0; }
    void onEnter(WuaDisplay &display) override;
    bool onButton(ButtonId id, ButtonEvent event) override;

protected:
    bool nextPass(String &text, Rgb &color) override;
    uint8_t passCount() const override { return _current.loops; }
    void onFinished() override { _active = false; }

private:
    struct Entry
    {
        char text[112];
        Rgb color;
        uint8_t loops;
    };

    Entry _queue[kQueueSize] = {};
    uint8_t _head = 0;
    uint8_t _count = 0;
    Entry _current = {};
    bool _active = false;
};
