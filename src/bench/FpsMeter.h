#pragma once

#include <Arduino.h>

// Frames-per-second meter with an optional moving average
// (ported unchanged in behavior from WuaDisplay-r1).
class FpsMeter
{
public:
    // intervalMs: measurement window. averageWindow: samples (0 = disabled).
    explicit FpsMeter(uint32_t intervalMs = 1000, uint8_t averageWindow = 0)
        : _interval(intervalMs), _avgWindow(averageWindow > kMaxWindow ? kMaxWindow : averageWindow) {}

    // Call once per rendered frame.
    void frame()
    {
        ++_frameCount;
        const uint32_t now = millis();
        if (now - _lastTime < _interval)
            return;
        uint32_t elapsed = now - _lastTime;
        if (elapsed == 0)
            elapsed = 1;
        _fpsCurrent = static_cast<uint16_t>((_frameCount * 1000UL) / elapsed);
        _frameCount = 0;
        _lastTime = now;
        _updated = true;
        updateAverage();
    }

    bool hasNewValue() const { return _updated; }
    void clearUpdated() { _updated = false; }
    uint16_t getFps() const { return _fpsCurrent; }
    uint16_t getAverageFps() const { return _avgWindow > 0 ? _fpsAverage : _fpsCurrent; }

private:
    static constexpr uint8_t kMaxWindow = 16;

    void updateAverage()
    {
        if (_avgWindow == 0)
            return;
        _avgBuffer[_avgIndex] = _fpsCurrent;
        _avgIndex = (_avgIndex + 1) % _avgWindow;
        if (_avgCount < _avgWindow)
            ++_avgCount;
        uint32_t sum = 0;
        for (uint8_t i = 0; i < _avgCount; ++i)
            sum += _avgBuffer[i];
        _fpsAverage = static_cast<uint16_t>(sum / _avgCount);
    }

    uint32_t _interval;
    uint32_t _lastTime = 0;
    uint32_t _frameCount = 0;
    uint16_t _fpsCurrent = 0;
    bool _updated = false;

    uint8_t _avgWindow;
    uint16_t _avgBuffer[kMaxWindow] = {0};
    uint8_t _avgCount = 0;
    uint8_t _avgIndex = 0;
    uint16_t _fpsAverage = 0;
};
