#include "net/HttpJson.h"

#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "config/Config.h"
#include "core/Logger.h"

namespace HttpJson
{
    int get(const String &url, JsonDocument &doc, const JsonDocument *filter)
    {
        WiFiClientSecure client;
        client.setInsecure();

        HTTPClient http;
        http.setConnectTimeout(Config::kHttpTimeoutMs);
        http.setTimeout(Config::kHttpTimeoutMs);
        http.setUserAgent("WuaStation-mini/" WUA_FIRMWARE_VERSION);
        // HTTP/1.0: no chunked encoding, so the body can be parsed as a stream.
        http.useHTTP10(true);

        if (!http.begin(client, url))
        {
            LOG_WARN("HTTP begin failed: %s", url.c_str());
            return HTTPC_ERROR_CONNECTION_REFUSED;
        }

        int code = http.GET();
        if (code == HTTP_CODE_OK)
        {
            const DeserializationError err =
                filter ? deserializeJson(doc, http.getStream(), DeserializationOption::Filter(*filter))
                       : deserializeJson(doc, http.getStream());
            if (err)
            {
                LOG_WARN("JSON error (%s): %s", err.c_str(), url.c_str());
                code = kJsonError;
            }
        }
        else
        {
            LOG_WARN("HTTP %d: %s", code, url.c_str());
        }

        http.end();
        return code;
    }
}
