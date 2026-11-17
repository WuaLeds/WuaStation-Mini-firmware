#include "wuastation.h"

#include <cmath>
#include <cstring>

#include "esphome/core/application.h"
#include "esphome/core/log.h"
#include "esphome/core/preferences.h"
#include "esphome/components/wifi/wifi_component.h"
#ifdef USE_API
#include "esphome/components/api/api_server.h"
#endif

#include "board/Board.h"
#include "config/Config.h"
#include "config/ScreenConfig.h"
#include "core/Logger.h"

#include "legacy_wifi.h"

namespace esphome::wuastation
{
    static const char *const TAG = "wuastation";

    void log_line(LogLevel level, const char *message)
    {
        switch (level)
        {
        case LogLevel::Error: ESP_LOGE(TAG, "%s", message); break;
        case LogLevel::Warn: ESP_LOGW(TAG, "%s", message); break;
        case LogLevel::Info: ESP_LOGI(TAG, "%s", message); break;
        case LogLevel::Debug: ESP_LOGD(TAG, "%s", message); break;
        }
    }

    WuaStation::WuaStation(web_server_base::WebServerBase *web) : station_(*this), web_(web), web_handler_(this)
    {
    }

    // -------------------------------------------------------------------------
    //  Life cycle
    // -------------------------------------------------------------------------
    void WuaStation::setup()
    {
        this->setup_access_point_();
        // Read now, applied once ESPHome's WiFi is set up (see loop()).
        this->legacy_wifi_pending_ = read_legacy_wifi(this->legacy_ssid_, sizeof(this->legacy_ssid_),
                                                      this->legacy_password_, sizeof(this->legacy_password_));

        this->station_.setListener(this);
        this->station_.setup();

        this->web_->add_handler(&this->web_handler_);
        this->web_->init();
    }

    void WuaStation::setup_access_point_()
    {
        // "WuaStation-7590": same name as firmware 0.3, shown on the matrix.
        snprintf(this->ap_name_, sizeof(this->ap_name_), "%s-%s", Config::kDeviceName, Board::deviceId());
        if (wifi::global_wifi_component == nullptr)
            return;
        wifi::WiFiAP ap;
        ap.set_ssid(this->ap_name_);
        ap.set_password(Config::kPortalPassword);
        wifi::global_wifi_component->set_ap(ap);
    }

    void WuaStation::migrate_legacy_wifi_()
    {
        auto *wifi = wifi::global_wifi_component;
        if (wifi == nullptr || !wifi->is_ready())
            return; // ESPHome's WiFi not set up yet
        this->legacy_wifi_pending_ = false;
        if (!wifi->has_sta())
        {
            ESP_LOGI(TAG, "Keeping the WiFi network saved by firmware 0.3");
            wifi->save_wifi_sta(this->legacy_ssid_, this->legacy_password_);
        }
        erase_legacy_wifi();
        memset(this->legacy_ssid_, 0, sizeof(this->legacy_ssid_));
        memset(this->legacy_password_, 0, sizeof(this->legacy_password_));
    }

    void WuaStation::loop()
    {
        if (this->legacy_wifi_pending_)
            this->migrate_legacy_wifi_();

        const bool connected = wifi::global_wifi_component != nullptr && wifi::global_wifi_component->is_connected();
        if (connected)
            this->offline_since_ = 0;
        else if (this->offline_since_ == 0)
            this->offline_since_ = millis() | 1u;

        this->station_.loop();
        this->publish_sensors_();

        if (!this->published_once_)
        {
            // Entities are set up by now: show them the station's state.
            this->published_once_ = true;
            this->publish_settings_();
#ifdef USE_LIGHT
            this->sync_light();
#endif
            this->onScreenChanged();
        }
    }

    void WuaStation::dump_config()
    {
        ESP_LOGCONFIG(TAG,
                      "WuaStation-mini:\n"
                      "  Firmware: %s\n"
                      "  Setup access point: %s",
                      WUA_FIRMWARE_VERSION, this->ap_name_);
    }

    // -------------------------------------------------------------------------
    //  Actions
    // -------------------------------------------------------------------------
    void WuaStation::show_message(const std::string &text, uint8_t loops)
    {
        this->station_.notify(text.c_str(), Theme::kInfo, loops);
    }

    void WuaStation::show_screen(const std::string &name)
    {
        ScreenId id;
        if (ScreenConfig::fromName(name.c_str(), id))
            this->station_.showScreen(id);
        else
            ESP_LOGW(TAG, "Unknown screen '%s'", name.c_str());
    }

    void WuaStation::run_on_loop(std::function<void()> &&f)
    {
        this->defer(std::move(f));
    }

    void WuaStation::restart_soon(uint32_t delay_ms)
    {
        this->set_timeout("restart", delay_ms, [this]() { this->restart(); });
    }

    void WuaStation::forget_wifi_soon(uint32_t delay_ms)
    {
        this->set_timeout("forget_wifi", delay_ms, [this]() { this->forgetWifi(); });
    }

    // -------------------------------------------------------------------------
    //  Host
    // -------------------------------------------------------------------------
    ::Host::NetState WuaStation::netState() const
    {
        auto *wifi = wifi::global_wifi_component;
        if (wifi == nullptr)
            return NetState::Offline;
        if (wifi->is_connected())
            return NetState::Connected;
        if (!wifi->has_sta() && !this->legacy_wifi_pending_)
            return wifi->is_ap_active() ? NetState::Setup : NetState::Connecting;
        if (this->offline_since_ != 0 && millis() - this->offline_since_ >= Config::kOfflineAfterMs)
            return NetState::Offline;
        return NetState::Connecting;
    }

    void WuaStation::ip(char *out, size_t size) const
    {
        if (size == 0)
            return;
        out[0] = '\0';
        auto *wifi = wifi::global_wifi_component;
        if (wifi == nullptr)
            return;
        char buf[network::IP_ADDRESS_BUFFER_SIZE];
        if (wifi->is_connected())
        {
            for (auto &address : wifi->wifi_sta_ip_addresses())
            {
                if (address.is_set() && address.is_ip4())
                {
                    address.str_to(buf);
                    strncpy(out, buf, size - 1);
                    out[size - 1] = '\0';
                    return;
                }
            }
        }
        else if (wifi->is_ap_active())
        {
            wifi->wifi_soft_ap_ip().str_to(buf);
            strncpy(out, buf, size - 1);
            out[size - 1] = '\0';
        }
    }

    const char *WuaStation::hostname() const
    {
        return App.get_name().c_str();
    }

    void WuaStation::ssid(char *out, size_t size) const
    {
        if (size == 0)
            return;
        out[0] = '\0';
        auto *wifi = wifi::global_wifi_component;
        if (wifi == nullptr || !wifi->is_connected())
            return;
        char buf[wifi::SSID_BUFFER_SIZE];
        strncpy(out, wifi->wifi_ssid_to(buf), size - 1);
        out[size - 1] = '\0';
    }

    int8_t WuaStation::rssi() const
    {
        auto *wifi = wifi::global_wifi_component;
        return wifi != nullptr && wifi->is_connected() ? wifi->wifi_rssi() : 0;
    }

    bool WuaStation::apActive() const
    {
        return wifi::global_wifi_component != nullptr && wifi::global_wifi_component->is_ap_active();
    }

    const char *WuaStation::apPassword() const
    {
        return Config::kPortalPassword;
    }

    bool WuaStation::homeAssistantConnected() const
    {
#ifdef USE_API
        return api::global_api_server != nullptr && api::global_api_server->is_connected();
#else
        return false;
#endif
    }

    void WuaStation::restart()
    {
        ESP_LOGI(TAG, "Restarting");
        App.safe_reboot();
    }

    void WuaStation::forgetWifi()
    {
        ESP_LOGW(TAG, "Forgetting the WiFi network");
        this->eraseHostData();
        App.safe_reboot();
    }

    void WuaStation::eraseHostData()
    {
        // ESPHome's stored data: WiFi network, entity states...
        global_preferences->reset();
        erase_legacy_wifi();
    }

    // -------------------------------------------------------------------------
    //  Station -> Home Assistant
    // -------------------------------------------------------------------------
    void WuaStation::onSettingsChanged()
    {
        this->publish_settings_();
#ifdef USE_LIGHT
        this->sync_light();
#endif
    }

    void WuaStation::publish_settings_()
    {
        const Settings &s = this->station_.settings().get();
#ifdef USE_TEXT
        if (this->message_text_ != nullptr &&
            (!this->message_text_->has_state() || this->message_text_->state != s.message))
            this->message_text_->publish_state(s.message);
#endif
#ifdef USE_SWITCH
        for (auto *sw : this->screen_switches_)
        {
            const bool on = ScreenConfig::isEnabled(s.screens, static_cast<ScreenId>(sw->get_screen()));
            if (!this->published_once_ || sw->state != on)
                sw->publish_state(on);
        }
#endif
    }

#ifdef USE_LIGHT
    void WuaStation::sync_light()
    {
        if (this->display_light_ == nullptr || this->display_light_->get_state() == nullptr)
            return;
        light::LightState *state = this->display_light_->get_state();
        const bool on = this->station_.displayOn();
        const float brightness = this->station_.settings().get().brightness / 255.0f;
        if (state->remote_values.is_on() == on &&
            (!on || std::fabs(state->remote_values.get_brightness() - brightness) < 0.003f))
            return;
        auto call = state->make_call();
        call.set_state(on);
        if (on)
            call.set_brightness(brightness);
        call.set_transition_length(0);
        call.perform();
    }
#endif

    void WuaStation::onButton(ButtonId id, ButtonEvent event)
    {
#ifdef USE_EVENT
        event::Event *target = this->button_events_[static_cast<uint8_t>(id) % 3];
        if (target == nullptr)
            return;
        if (event == ButtonEvent::Click)
            target->trigger("click");
        else if (event == ButtonEvent::LongPress)
            target->trigger("long_press");
#else
        (void)id;
        (void)event;
#endif
    }

    void WuaStation::onScreenChanged()
    {
#ifdef USE_SELECT
        ScreenId id;
        if (this->screen_select_ != nullptr && this->station_.currentScreen(id))
        {
            const size_t index = static_cast<size_t>(id);
            const auto active = this->screen_select_->active_index();
            if (!active.has_value() || *active != index)
                this->screen_select_->publish_state(index);
        }
#endif
    }

    void WuaStation::publish_sensors_()
    {
#ifdef USE_SENSOR
        const SensorManager &sensors = this->station_.sensors();
        if (sensors.version() != this->sensor_version_ || !this->published_once_)
        {
            this->sensor_version_ = sensors.version();
            const bool has = sensors.hasReading();
            const SensorReading &r = sensors.reading();
            if (this->indoor_temperature_ != nullptr)
                this->indoor_temperature_->publish_state(has ? r.temperatureC : NAN);
            if (this->indoor_humidity_ != nullptr)
                this->indoor_humidity_->publish_state(has && r.hasHumidity ? r.humidity : NAN);
            if (this->indoor_pressure_ != nullptr)
                this->indoor_pressure_->publish_state(has && r.hasPressure ? r.pressureHpa : NAN);
        }

        const OpenMeteoWeatherProvider &weather = this->station_.weather();
        if (weather.version() != this->weather_version_)
        {
            WeatherData w;
            this->weather_version_ = weather.read(w);
            if (w.valid)
            {
                if (this->outdoor_temperature_ != nullptr)
                    this->outdoor_temperature_->publish_state(w.temperatureC);
                if (this->outdoor_humidity_ != nullptr)
                    this->outdoor_humidity_->publish_state(w.humidity);
                if (this->rain_probability_ != nullptr)
                    this->rain_probability_->publish_state(w.hasRainChance ? w.rainChance : NAN);
                if (this->uv_index_ != nullptr)
                    this->uv_index_->publish_state(w.hasUv ? w.uvMax : NAN);
                if (this->air_quality_index_ != nullptr)
                    this->air_quality_index_->publish_state(
                        w.hasAirQuality ? (weather.europeanAqi() ? w.europeanAqi : w.usAqi) : NAN);
            }
        }
#endif
    }

    // -------------------------------------------------------------------------
    //  Home Assistant -> station
    // -------------------------------------------------------------------------
#ifdef USE_TEXT
    void MessageText::control(const std::string &value)
    {
        this->parent_->station().setMessage(value.c_str());
        this->publish_state(this->parent_->station().settings().get().message);
    }
#endif

#ifdef USE_SELECT
    void ScreenSelect::control(size_t index)
    {
        this->parent_->station().showScreen(static_cast<ScreenId>(index));
        // Report what is really on display (a hidden screen cannot be shown).
        ScreenId id;
        if (this->parent_->station().currentScreen(id))
            this->publish_state(static_cast<size_t>(id));
    }
#endif

#ifdef USE_SWITCH
    void ScreenSwitch::write_state(bool state)
    {
        auto &station = this->parent_->station();
        station.setScreenEnabled(static_cast<ScreenId>(this->screen_), state);
        this->publish_state(
            ScreenConfig::isEnabled(station.settings().get().screens, static_cast<ScreenId>(this->screen_)));
    }
#endif

#ifdef USE_BUTTON
    void StationButton::press_action()
    {
        switch (this->action_)
        {
        case NEXT_SCREEN: this->parent_->next_screen(); break;
        case FACTORY_RESET: this->parent_->factory_reset(); break;
        }
    }
#endif

#ifdef USE_LIGHT
    light::LightTraits DisplayLight::get_traits()
    {
        light::LightTraits traits;
        traits.set_supported_color_modes({light::ColorMode::BRIGHTNESS});
        return traits;
    }

    void DisplayLight::write_state(light::LightState *state)
    {
        if (!this->booted_)
        {
            // The light restoring itself at boot: the station keeps its own
            // stored brightness and the light follows it (sync_light).
            this->booted_ = true;
            return;
        }
        auto &station = this->parent_->station();
        if (!state->current_values.is_on())
        {
            station.setDisplayOn(false);
            return;
        }
        long level = std::lround(state->current_values.get_brightness() * 255.0f);
        level = level < 1 ? 1 : (level > 255 ? 255 : level);
        if (!station.displayOn() || static_cast<uint8_t>(level) != station.settings().get().brightness)
            station.setBrightness(static_cast<uint8_t>(level));
    }
#endif
}

// The firmware core's log hook (core/Logger.h).
void wua_log_write(LogLevel level, const char *message)
{
    esphome::wuastation::log_line(level, message);
}
