#pragma once

#include <cstddef>

namespace esphome::wuastation
{
    // WiFi network saved by firmware 0.3 and earlier, in ESP-IDF's own NVS
    // storage ("nvs.net80211"), which ESPHome does not read. Moving it into
    // ESPHome keeps an updated station on its network.
    bool read_legacy_wifi(char *ssid, size_t ssid_size, char *password, size_t password_size);

    // Remove it, so a factory reset really forgets the network.
    void erase_legacy_wifi();
}
