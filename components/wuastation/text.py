"""The custom scrolling message (empty hides the message screen)."""

import esphome.codegen as cg
from esphome.components import text
import esphome.config_validation as cv

from . import CONF_WUASTATION_ID, WuaStation, wuastation_ns

DEPENDENCIES = ["wuastation"]

MessageText = wuastation_ns.class_("MessageText", text.Text)

CONF_MESSAGE = "message"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_WUASTATION_ID): cv.use_id(WuaStation),
        cv.Optional(CONF_MESSAGE): text.text_schema(
            MessageText, icon="mdi:message-text", mode="TEXT"
        ),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_WUASTATION_ID])
    if conf := config.get(CONF_MESSAGE):
        var = await text.new_text(conf, min_length=0, max_length=127)
        cg.add(var.set_parent(parent))
        cg.add(parent.set_message_text(var))
