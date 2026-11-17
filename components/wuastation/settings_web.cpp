#include "settings_web.h"

#include <ArduinoJson.h>
#include <memory>

#include "esphome/core/defines.h"
#include "esphome/core/log.h"
#ifdef USE_CAPTIVE_PORTAL
#include "esphome/components/captive_portal/captive_portal.h"
#endif

#include "core/Settings.h"
#include "web/SettingsJson.h"

#include "wuastation.h"
#include "wuastation_settings_page.h" // generated from web/settings.html by __init__.py

namespace esphome::wuastation
{
    static const char *const TAG = "wuastation.web";
    static constexpr size_t MAX_BODY = 4096;

    bool SettingsWebHandler::canHandle(AsyncWebServerRequest *request) const
    {
#ifdef USE_CAPTIVE_PORTAL
        // WiFi setup first: the captive portal answers while it is open.
        if (captive_portal::global_captive_portal != nullptr && captive_portal::global_captive_portal->is_active())
            return false;
#endif
        char buf[AsyncWebServerRequest::URL_BUF_SIZE];
        const StringRef url = request->url_to(buf);
        if (request->method() == HTTP_GET)
            return url == "/" || url == "/index.html" || url == "/api/settings" || url == "/api/status";
        if (request->method() == HTTP_POST)
            return url == "/api/settings" || url == "/api/brightness" || url == "/api/restart" ||
                   url == "/api/wifi/forget";
        return false;
    }

    void SettingsWebHandler::handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index,
                                        size_t total)
    {
        (void)request;
        if (index == 0)
        {
            this->body_.clear();
            if (total <= MAX_BODY)
                this->body_.reserve(total);
        }
        if (this->body_.size() + len <= MAX_BODY)
            this->body_.append(reinterpret_cast<const char *>(data), len);
    }

    void SettingsWebHandler::handleRequest(AsyncWebServerRequest *request)
    {
        char buf[AsyncWebServerRequest::URL_BUF_SIZE];
        const StringRef url = request->url_to(buf);

        if (request->method() == HTTP_GET)
        {
            if (url == "/api/settings")
                this->send_settings_(request);
            else if (url == "/api/status")
                this->send_status_(request);
            else
                this->send_page_(request);
            return;
        }

        if (url == "/api/settings")
        {
            this->save_settings_(request);
        }
        else if (url == "/api/brightness")
        {
            this->set_brightness_(request);
        }
        else if (url == "/api/restart")
        {
            WuaStation *parent = this->parent_;
            this->send_ok_(request);
            parent->run_on_loop([parent]() { parent->restart_soon(800); });
        }
        else if (url == "/api/wifi/forget")
        {
            WuaStation *parent = this->parent_;
            this->send_ok_(request);
            parent->run_on_loop([parent]() { parent->forget_wifi_soon(800); });
        }
        this->body_.clear();
    }

    void SettingsWebHandler::send_page_(AsyncWebServerRequest *request)
    {
        auto *response =
            request->beginResponse(200, "text/html; charset=utf-8", SETTINGS_PAGE_GZ, SETTINGS_PAGE_GZ_SIZE);
        response->addHeader("Content-Encoding", "gzip");
        response->addHeader("Cache-Control", "no-cache");
        request->send(response);
    }

    void SettingsWebHandler::send_status_(AsyncWebServerRequest *request)
    {
        JsonDocument doc;
        auto &station = this->parent_->station();
        SettingsJson::writeStatus(doc.to<JsonObject>(), *this->parent_, station.time(), station.sensors());
        std::string out;
        serializeJson(doc, out);
        this->send_json_(request, 200, out);
    }

    void SettingsWebHandler::send_settings_(AsyncWebServerRequest *request)
    {
        JsonDocument doc;
        auto &station = this->parent_->station();
        JsonObject root = doc.to<JsonObject>();
        // On the heap (small task stack). The document references its strings:
        // it must outlive serializeJson().
        auto settings = std::make_unique<Settings>();
        station.settings().snapshotInto(*settings);
        SettingsJson::writeStatus(root, *this->parent_, station.time(), station.sensors());
        SettingsJson::writeSettings(root, *settings);
        std::string out;
        serializeJson(doc, out);
        this->send_json_(request, 200, out);
    }

    void SettingsWebHandler::save_settings_(AsyncWebServerRequest *request)
    {
        // Settings are ~0.5 KB: kept on the heap, the web server task has a
        // 4.5 KB stack.
        auto before = std::make_unique<Settings>();
        auto after = std::make_shared<Settings>();
        this->parent_->station().settings().snapshotInto(*before);
        *after = *before;
        {
            JsonDocument in;
            if (this->body_.empty() || deserializeJson(in, this->body_) ||
                !SettingsJson::read(in.as<JsonVariantConst>(), *after))
            {
                this->send_error_(request, 400, "invalid JSON body");
                return;
            }
        }
        SettingsStore::sanitize(*after);
        const bool restart = SettingsStore::needsRestart(*before, *after);

        WuaStation *parent = this->parent_;
        parent->run_on_loop([parent, after]() { parent->station().applySettings(*after); });
        ESP_LOGI(TAG, "Settings received from the settings page%s", restart ? " (restart)" : "");

        JsonDocument out;
        out["ok"] = true;
        out["restart"] = restart;
        std::string json;
        serializeJson(out, json);
        this->send_json_(request, 200, json);
    }

    void SettingsWebHandler::set_brightness_(AsyncWebServerRequest *request)
    {
        if (!request->hasArg("value"))
        {
            this->send_error_(request, 400, "missing value");
            return;
        }
        long value = strtol(request->arg("value").c_str(), nullptr, 10);
        value = value < 1 ? 1 : (value > 255 ? 255 : value);
        WuaStation *parent = this->parent_;
        const uint8_t level = static_cast<uint8_t>(value);
        parent->run_on_loop([parent, level]() { parent->station().setBrightness(level); });
        this->send_ok_(request);
    }

    void SettingsWebHandler::send_json_(AsyncWebServerRequest *request, int code, const std::string &json)
    {
        auto *response = request->beginResponse(code, "application/json", json);
        response->addHeader("Cache-Control", "no-store");
        request->send(response);
    }

    void SettingsWebHandler::send_ok_(AsyncWebServerRequest *request)
    {
        this->send_json_(request, 200, "{\"ok\":true}");
    }

    void SettingsWebHandler::send_error_(AsyncWebServerRequest *request, int code, const char *message)
    {
        JsonDocument doc;
        doc["ok"] = false;
        doc["error"] = message;
        std::string json;
        serializeJson(doc, json);
        this->send_json_(request, code, json);
    }
}
