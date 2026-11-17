"""One switch per screen: whether it is part of the rotation."""

import esphome.codegen as cg
from esphome.components import switch
import esphome.config_validation as cv
from esphome.const import ENTITY_CATEGORY_CONFIG

from . import CONF_WUASTATION_ID, WuaStation, wuastation_ns

DEPENDENCIES = ["wuastation"]

ScreenSwitch = wuastation_ns.class_("ScreenSwitch", switch.Switch)

# key -> (ScreenId, icon); same order as ScreenId in ScreenConfig.h.
SCREENS = {
    "time_screen": (0, "mdi:clock-outline"),
    "date_screen": (1, "mdi:calendar"),
    "weather_screen": (2, "mdi:weather-partly-cloudy"),
    "forecast_screen": (3, "mdi:weather-cloudy-clock"),
    "indoor_screen": (4, "mdi:home-thermometer"),
    "ticker_screen": (5, "mdi:chart-line"),
    "message_screen": (6, "mdi:message-text"),
    "celebration_screen": (7, "mdi:party-popper"),
    "timer_screen": (8, "mdi:timer-outline"),
}

# The station owns the state: no restore, it is published at boot.
CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_WUASTATION_ID): cv.use_id(WuaStation),
        **{
            cv.Optional(key): switch.switch_schema(
                ScreenSwitch,
                default_restore_mode="DISABLED",
                entity_category=ENTITY_CATEGORY_CONFIG,
                icon=icon,
            )
            for key, (_, icon) in SCREENS.items()
        },
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_WUASTATION_ID])
    for key, (screen, _) in SCREENS.items():
        if conf := config.get(key):
            var = await switch.new_switch(conf)
            cg.add(var.set_parent(parent))
            cg.add(var.set_screen(screen))
            cg.add(parent.add_screen_switch(var))
