#pragma once

// =============================================================================
//  Home Assistant entities that control the station. Each one forwards to the
//  WuaStation component, which applies the change on the main loop and
//  mirrors every change back (whatever its origin: Home Assistant, the
//  settings page or the buttons).
// =============================================================================

#include "esphome/core/defines.h"
#include "esphome/core/helpers.h"

#ifdef USE_TEXT
#include "esphome/components/text/text.h"
#endif
#ifdef USE_SELECT
#include "esphome/components/select/select.h"
#endif
#ifdef USE_SWITCH
#include "esphome/components/switch/switch.h"
#endif
#ifdef USE_BUTTON
#include "esphome/components/button/button.h"
#endif
#ifdef USE_LIGHT
#include "esphome/components/light/light_output.h"
#include "esphome/components/light/light_state.h"
#endif

namespace esphome::wuastation
{
    class WuaStation;

#ifdef USE_TEXT
    // The custom scrolling message (empty hides the message screen).
    class MessageText : public text::Text, public Parented<WuaStation>
    {
    protected:
        void control(const std::string &value) override;
    };
#endif

#ifdef USE_SELECT
    // Screen on display; selecting one jumps to it.
    class ScreenSelect : public select::Select, public Parented<WuaStation>
    {
    protected:
        void control(size_t index) override;
    };
#endif

#ifdef USE_SWITCH
    // Whether one screen is part of the rotation.
    class ScreenSwitch : public switch_::Switch, public Parented<WuaStation>
    {
    public:
        void set_screen(uint8_t screen) { this->screen_ = screen; }
        uint8_t get_screen() const { return this->screen_; }

    protected:
        void write_state(bool state) override;
        uint8_t screen_{0};
    };
#endif

#ifdef USE_BUTTON
    class StationButton : public button::Button, public Parented<WuaStation>
    {
    public:
        enum Action : uint8_t
        {
            NEXT_SCREEN = 0,
            FACTORY_RESET = 1,
        };
        void set_action(uint8_t action) { this->action_ = action; }

    protected:
        void press_action() override;
        uint8_t action_{NEXT_SCREEN};
    };
#endif

#ifdef USE_LIGHT
    // The matrix as a dimmable light: on/off and brightness.
    class DisplayLight : public light::LightOutput, public Parented<WuaStation>
    {
    public:
        light::LightTraits get_traits() override;
        void setup_state(light::LightState *state) override { this->state_ = state; }
        void write_state(light::LightState *state) override;
        light::LightState *get_state() const { return this->state_; }

    protected:
        light::LightState *state_{nullptr};
        bool booted_{false};
    };
#endif
}
