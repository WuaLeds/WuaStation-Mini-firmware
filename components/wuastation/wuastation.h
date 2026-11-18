#pragma once

// =============================================================================
//  WuaStation -- the WuaStation-mini firmware running inside ESPHome.
//
//  The firmware core (lib/WuaStationCore: screens, internet data, sensors,
//  settings) is the same with or without Home Assistant. This component
//  hosts it: it gives it the network state, serves the settings page on
//  ESPHome's web server and mirrors the station to Home Assistant entities.
// =============================================================================

#include <functional>
#include <string>
#include <vector>

#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include "esphome/components/web_server_base/web_server_base.h"

#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif
#ifdef USE_EVENT
#include "esphome/components/event/event.h"
#endif

#include "app/Host.h"
#include "app/Station.h"

#include "entities.h"
#include "settings_web.h"

namespace esphome::wuastation
{
    class WuaStation : public Component, public ::Host, public ::Station::Listener
    {
    public:
        explicit WuaStation(web_server_base::WebServerBase *web);

        // Right before WiFi: the display shows the boot screen while the
        // network comes up, and the setup access point gets our name.
        float get_setup_priority() const override { return setup_priority::WIFI + 1.0f; }
        void setup() override;
        void loop() override;
        void dump_config() override;

        ::Station &station() { return this->station_; }

        // ---- actions (YAML lambdas, Home Assistant actions) ----
        void show_message(const std::string &text, uint8_t loops = 2);
        void show_screen(const std::string &name);
        void next_screen() { this->station_.nextScreen(); }
        void factory_reset() { this->station_.factoryReset(); }

        // For the web server task: run `f` on the main loop.
        void run_on_loop(std::function<void()> &&f);
        void restart_soon(uint32_t delay_ms);
        void forget_wifi_soon(uint32_t delay_ms);

        // ---- Host ----
        NetState netState() const override;
        void ip(char *out, size_t size) const override;
        const char *hostname() const override;
        void ssid(char *out, size_t size) const override;
        int8_t rssi() const override;
        bool apActive() const override;
        const char *apName() const override { return this->ap_name_; }
        const char *apPassword() const override;
        bool homeAssistantConnected() const override;
        void restart() override;
        void forgetWifi() override;
        void eraseHostData() override;

        // ---- Station::Listener ----
        void onSettingsChanged() override;
        void onButton(ButtonId id, ButtonEvent event) override;
        void onScreenChanged() override;

        // ---- entities (code generation) ----
#ifdef USE_SENSOR
        void set_indoor_temperature_sensor(sensor::Sensor *s) { this->indoor_temperature_ = s; }
        void set_indoor_humidity_sensor(sensor::Sensor *s) { this->indoor_humidity_ = s; }
        void set_indoor_pressure_sensor(sensor::Sensor *s) { this->indoor_pressure_ = s; }
        void set_outdoor_temperature_sensor(sensor::Sensor *s) { this->outdoor_temperature_ = s; }
        void set_outdoor_humidity_sensor(sensor::Sensor *s) { this->outdoor_humidity_ = s; }
        void set_rain_probability_sensor(sensor::Sensor *s) { this->rain_probability_ = s; }
        void set_uv_index_sensor(sensor::Sensor *s) { this->uv_index_ = s; }
        void set_air_quality_index_sensor(sensor::Sensor *s) { this->air_quality_index_ = s; }
#endif
#ifdef USE_TEXT
        void set_message_text(MessageText *text) { this->message_text_ = text; }
#endif
#ifdef USE_SELECT
        void set_screen_select(ScreenSelect *select) { this->screen_select_ = select; }
#endif
#ifdef USE_SWITCH
        void add_screen_switch(ScreenSwitch *sw) { this->screen_switches_.push_back(sw); }
#endif
#ifdef USE_LIGHT
        void set_display_light(DisplayLight *light) { this->display_light_ = light; }
        // Bring the light entity in line with the station (after a change
        // from the buttons, the settings page or the boot).
        void sync_light();
#endif
#ifdef USE_EVENT
        void set_button_event(uint8_t button, event::Event *event) { this->button_events_[button % 3] = event; }
#endif

    protected:
        void setup_access_point_();
        void migrate_legacy_wifi_();
        void update_bluetooth_();
        void publish_settings_();
        void publish_sensors_();

        ::Station station_;
        web_server_base::WebServerBase *web_;
        SettingsWebHandler web_handler_;

        char ap_name_[24]{};
        uint32_t offline_since_{0};
        int8_t bluetooth_on_{-1}; // -1 = not decided yet
        bool legacy_wifi_pending_{false};
        char legacy_ssid_[33]{};
        char legacy_password_[65]{};

        uint32_t sensor_version_{0};
        uint32_t weather_version_{0};
        bool published_once_{false};

#ifdef USE_SENSOR
        sensor::Sensor *indoor_temperature_{nullptr};
        sensor::Sensor *indoor_humidity_{nullptr};
        sensor::Sensor *indoor_pressure_{nullptr};
        sensor::Sensor *outdoor_temperature_{nullptr};
        sensor::Sensor *outdoor_humidity_{nullptr};
        sensor::Sensor *rain_probability_{nullptr};
        sensor::Sensor *uv_index_{nullptr};
        sensor::Sensor *air_quality_index_{nullptr};
#endif
#ifdef USE_TEXT
        MessageText *message_text_{nullptr};
#endif
#ifdef USE_SELECT
        ScreenSelect *screen_select_{nullptr};
#endif
#ifdef USE_SWITCH
        std::vector<ScreenSwitch *> screen_switches_;
#endif
#ifdef USE_LIGHT
        DisplayLight *display_light_{nullptr};
#endif
#ifdef USE_EVENT
        event::Event *button_events_[3]{nullptr, nullptr, nullptr};
#endif
    };
}
