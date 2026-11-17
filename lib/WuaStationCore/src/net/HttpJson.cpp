#include "net/HttpJson.h"

#include "config/Config.h"
#include "core/Logger.h"
#include "esp_crt_bundle.h"
#include "esp_http_client.h"

namespace
{
    // ArduinoJson reader over an open esp_http_client response (chunked
    // transfer encoding is decoded by esp_http_client_read).
    class ResponseReader
    {
    public:
        explicit ResponseReader(esp_http_client_handle_t client) : _client(client) {}

        int read()
        {
            if (_pos >= _len && !fill())
                return -1;
            return static_cast<uint8_t>(_buf[_pos++]);
        }

        size_t readBytes(char *out, size_t length)
        {
            size_t done = 0;
            while (done < length)
            {
                if (_pos >= _len && !fill())
                    break;
                size_t chunk = static_cast<size_t>(_len - _pos);
                if (chunk > length - done)
                    chunk = length - done;
                memcpy(out + done, _buf + _pos, chunk);
                _pos += static_cast<int>(chunk);
                done += chunk;
            }
            return done;
        }

    private:
        bool fill()
        {
            if (_eof)
                return false;
            const int n = esp_http_client_read(_client, _buf, sizeof(_buf));
            if (n <= 0)
            {
                _eof = true;
                return false;
            }
            _len = n;
            _pos = 0;
            return true;
        }

        esp_http_client_handle_t _client;
        char _buf[256];
        int _len = 0;
        int _pos = 0;
        bool _eof = false;
    };
}

namespace HttpJson
{
    int get(const char *url, JsonDocument &doc, const JsonDocument *filter)
    {
        esp_http_client_config_t config = {};
        config.url = url;
        config.method = HTTP_METHOD_GET;
        config.timeout_ms = static_cast<int>(Config::kHttpTimeoutMs);
        config.crt_bundle_attach = esp_crt_bundle_attach;
        config.user_agent = "WuaStation-mini/" WUA_FIRMWARE_VERSION;
        config.buffer_size = 1024; // response headers
        config.buffer_size_tx = 512;
        config.keep_alive_enable = false;

        esp_http_client_handle_t client = esp_http_client_init(&config);
        if (!client)
            return kConnectError;

        int code = kConnectError;
        const esp_err_t err = esp_http_client_open(client, 0);
        if (err != ESP_OK)
        {
            LOG_WARN("HTTP connect failed (%s): %s", esp_err_to_name(err), url);
        }
        else
        {
            esp_http_client_fetch_headers(client);
            code = esp_http_client_get_status_code(client);
            if (code == kOk)
            {
                ResponseReader reader(client);
                const DeserializationError jsonErr =
                    filter ? deserializeJson(doc, reader, DeserializationOption::Filter(*filter))
                           : deserializeJson(doc, reader);
                if (jsonErr)
                {
                    LOG_WARN("JSON error (%s): %s", jsonErr.c_str(), url);
                    code = kJsonError;
                }
            }
            else
            {
                LOG_WARN("HTTP %d: %s", code, url);
            }
            esp_http_client_close(client);
        }
        esp_http_client_cleanup(client);
        return code;
    }
}
