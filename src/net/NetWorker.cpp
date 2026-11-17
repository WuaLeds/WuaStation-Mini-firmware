#include "net/NetWorker.h"

#include <WiFi.h>
#include "config/Config.h"
#include "core/Logger.h"

bool NetWorker::add(INetJob *job)
{
    if (!job || _count >= kMaxJobs || _task)
        return false;
    _jobs[_count] = job;
    _dueMs[_count] = 0; // run as soon as the network is up
    ++_count;
    return true;
}

void NetWorker::begin()
{
    if (_task || _count == 0)
        return;
    // Same priority as the Arduino loop task: FreeRTOS time-slices them, so a
    // TLS handshake shares the CPU instead of freezing the display.
    xTaskCreate(&NetWorker::taskEntry, "net", Config::kNetTaskStackBytes, this, 1, &_task);
}

void NetWorker::taskEntry(void *self)
{
    static_cast<NetWorker *>(self)->taskLoop();
}

void NetWorker::taskLoop()
{
    LOG_INFO("Network task started (%u jobs)", _count);
    for (;;)
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            vTaskDelay(pdMS_TO_TICKS(500));
            continue;
        }

        for (uint8_t i = 0; i < _count; ++i)
        {
            INetJob *job = _jobs[i];
            if (!job->enabled())
                continue;
            // Signed difference handles the millis() wrap-around.
            if (static_cast<int32_t>(millis() - _dueMs[i]) < 0)
                continue;

            const uint32_t delayMs = job->run();
            _dueMs[i] = millis() + delayMs;
            LOG_DEBUG("Job %s done, next in %lu ms (heap %u)", job->name(), static_cast<unsigned long>(delayMs),
                      ESP.getFreeHeap());
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
