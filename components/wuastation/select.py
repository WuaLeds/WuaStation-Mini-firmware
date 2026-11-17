"""Screen on display; selecting one jumps to it."""

import esphome.codegen as cg
from esphome.components import select
import esphome.config_validation as cv

from . import CONF_WUASTATION_ID, WuaStation, wuastation_ns

DEPENDENCIES = ["wuastation"]

ScreenSelect = wuastation_ns.class_("ScreenSelect", select.Select)

CONF_SCREEN = "screen"

# Same order as ScreenId (lib/WuaStationCore/src/config/ScreenConfig.h).
SCREEN_TITLES = [
    "Time",
    "Date",
    "Weather",
    "Forecast",
    "Indoor",
    "Ticker",
    "Message",
    "Celebration",
    "Timer",
]

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_WUASTATION_ID): cv.use_id(WuaStation),
        cv.Optional(CONF_SCREEN): select.select_schema(
            ScreenSelect, icon="mdi:monitor-shimmer"
        ),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_WUASTATION_ID])
    if conf := config.get(CONF_SCREEN):
        var = await select.new_select(conf, options=SCREEN_TITLES)
        cg.add(var.set_parent(parent))
        cg.add(parent.set_screen_select(var))
