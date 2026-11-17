"""The physical buttons as Home Assistant events (click, long_press)."""

import esphome.codegen as cg
from esphome.components import event
import esphome.config_validation as cv

from . import CONF_WUASTATION_ID, WuaStation

DEPENDENCIES = ["wuastation"]

EVENT_TYPES = ["click", "long_press"]

# key -> ButtonId (lib/WuaStationCore/src/input/InputEvents.h)
BUTTONS = {
    "up_button": 0,
    "down_button": 1,
    "touch_button": 2,
}

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_WUASTATION_ID): cv.use_id(WuaStation),
        **{
            cv.Optional(key): event.event_schema(
                event.Event, device_class="button", icon="mdi:gesture-tap-button"
            )
            for key in BUTTONS
        },
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_WUASTATION_ID])
    for key, button_id in BUTTONS.items():
        if conf := config.get(key):
            var = await event.new_event(conf, event_types=EVENT_TYPES)
            cg.add(parent.set_button_event(button_id, var))
