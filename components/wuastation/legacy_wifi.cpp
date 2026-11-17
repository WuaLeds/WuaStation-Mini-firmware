#include "legacy_wifi.h"

#include <cstdint>
#include <cstring>

#include "nvs.h"

namespace esphome::wuastation
{
    namespace
    {
        constexpr const char *kNamespace = "nvs.net80211";
        constexpr const char *kKeySsid = "sta.ssid";     // wifi_ssid_t: uint32 length + 32 bytes
        constexpr const char *kKeyPassword = "sta.pswd"; // char[65], NUL-terminated
    }

    bool read_legacy_wifi(char *ssid, size_t ssid_size, char *password, size_t password_size)
    {
        nvs_handle_t handle;
        if (nvs_open(kNamespace, NVS_READONLY, &handle) != ESP_OK)
            return false;

        bool ok = false;
        uint8_t raw[36] = {};
        size_t length = sizeof(raw);
        if (nvs_get_blob(handle, kKeySsid, raw, &length) == ESP_OK && length == sizeof(raw))
        {
            uint32_t ssidLength = 0;
            memcpy(&ssidLength, raw, sizeof(ssidLength));
            if (ssidLength > 0 && ssidLength <= 32 && ssidLength < ssid_size)
            {
                memcpy(ssid, raw + 4, ssidLength);
                ssid[ssidLength] = '\0';
                ok = true;
            }
        }

        if (ok)
        {
            char secret[65] = {};
            length = sizeof(secret);
            if (nvs_get_blob(handle, kKeyPassword, secret, &length) != ESP_OK)
                secret[0] = '\0'; // open network
            secret[sizeof(secret) - 1] = '\0';
            strncpy(password, secret, password_size - 1);
            password[password_size - 1] = '\0';
            memset(secret, 0, sizeof(secret));
        }

        nvs_close(handle);
        return ok;
    }

    void erase_legacy_wifi()
    {
        nvs_handle_t handle;
        if (nvs_open(kNamespace, NVS_READWRITE, &handle) != ESP_OK)
            return;
        nvs_erase_key(handle, kKeySsid);
        nvs_erase_key(handle, kKeyPassword);
        nvs_commit(handle);
        nvs_close(handle);
    }
}
