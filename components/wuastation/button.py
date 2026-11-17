"""Station buttons: next screen, factory reset."""

import esphome.codegen as cg
from esphome.components import button
import esphome.config_validation as cv
from esphome.const import ENTITY_CATEGORY_CONFIG

from . import CONF_WUASTATION_ID, WuaStation, wuastation_ns

DEPENDENCIES = ["wuastation"]

StationButton = wuastation_ns.class_("StationButton", button.Button)

# key -> (StationButton::Action, schema)
BUTTONS = {
    "next_screen": (0, button.button_schema(StationButton, icon="mdi:skip-next")),
    "factory_reset": (
        1,
        button.button_schema(
            StationButton,
            icon="mdi:restart-alert",
            entity_category=ENTITY_CATEGORY_CONFIG,
        ),
    ),
}

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_WUASTATION_ID): cv.use_id(WuaStation),
        **{cv.Optional(key): schema for key, (_, schema) in BUTTONS.items()},
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_WUASTATION_ID])
    for key, (action, _) in BUTTONS.items():
        if conf := config.get(key):
            var = await button.new_button(conf)
            cg.add(var.set_parent(parent))
            cg.add(var.set_action(action))
