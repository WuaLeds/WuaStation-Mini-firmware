#pragma once

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// =============================================================================
//  Background network jobs.
//
//  Every internet data source (ticker, weather...) is an INetJob. A single
//  low-priority FreeRTOS task runs the jobs one after another whenever WiFi is
//  up, so HTTPS requests never block the display loop and only one TLS
//  session is alive at a time (the ESP32-C3 has limited RAM).
//
//  Jobs publish their results through SharedState<T>; the main loop reads
//  snapshots. Jobs must be configured before NetWorker::begin().
// =============================================================================
class INetJob
{
public:
    virtual ~INetJob() = default;

    virtual const char *name() const = 0;

    // Disabled jobs are skipped (e.g. no ticker symbols configured).
    virtual bool enabled() const { return true; }

    // Do the (blocking) network work. Returns the delay until the next run.
    virtual uint32_t run() = 0;
};

class NetWorker
{
public:
    static constexpr uint8_t kMaxJobs = 4;

    bool add(INetJob *job);
    void begin();

private:
    static void taskEntry(void *self);
    void taskLoop();

    INetJob *_jobs[kMaxJobs] = {};
    uint32_t _dueMs[kMaxJobs] = {};
    uint8_t _count = 0;
    TaskHandle_t _task = nullptr;
};
