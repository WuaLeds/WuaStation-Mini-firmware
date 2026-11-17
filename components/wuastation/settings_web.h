#pragma once

#include <string>

#include "esphome/components/web_server_base/web_server_base.h"

// =============================================================================
//  The Wualeds settings page (web/settings.html, gzipped at build time) and
//  its JSON API, on ESPHome's web server:
//
//    GET  /                   settings page
//    GET  /api/settings       every setting + device status
//    GET  /api/status         device status only (polled by the page)
//    POST /api/settings       JSON body: settings to apply
//    POST /api/brightness     value=1..255, live
//    POST /api/restart
//    POST /api/wifi/forget    forget the network, reopen the setup access point
//
//  While the setup access point is open for the first time, every request
//  goes to ESPHome's captive portal instead (WiFi first).
//
//  Requests are served on the web server's own task: they read a snapshot of
//  the settings and hand every change to the main loop with defer().
// =============================================================================
namespace esphome::wuastation
{
    class WuaStation;

    class SettingsWebHandler : public AsyncWebHandler
    {
    public:
        explicit SettingsWebHandler(WuaStation *parent) : parent_(parent) {}

        bool canHandle(AsyncWebServerRequest *request) const override;
        void handleRequest(AsyncWebServerRequest *request) override;
        void handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) override;
        bool isRequestHandlerTrivial() const override { return false; }

    protected:
        void send_page_(AsyncWebServerRequest *request);
        void send_settings_(AsyncWebServerRequest *request);
        void send_status_(AsyncWebServerRequest *request);
        void save_settings_(AsyncWebServerRequest *request);
        void set_brightness_(AsyncWebServerRequest *request);
        void send_json_(AsyncWebServerRequest *request, int code, const std::string &json);
        void send_ok_(AsyncWebServerRequest *request);
        void send_error_(AsyncWebServerRequest *request, int code, const char *message);

        WuaStation *parent_;
        std::string body_; // POST body being received (one request at a time)
    };
}
