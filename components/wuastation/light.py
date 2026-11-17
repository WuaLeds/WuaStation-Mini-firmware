"""The LED matrix as a dimmable light: on/off and brightness."""

import esphome.codegen as cg
from esphome.components import light
import esphome.config_validation as cv
from esphome.const import CONF_DEFAULT_TRANSITION_LENGTH, CONF_OUTPUT_ID

from . import CONF_WUASTATION_ID, WuaStation, wuastation_ns

DEPENDENCIES = ["wuastation"]

DisplayLight = wuastation_ns.class_("DisplayLight", light.LightOutput)

# The station stores its own brightness: the light follows it at boot (its
# own restored value is ignored) and changes apply at once.
CONFIG_SCHEMA = light.light_schema(
    DisplayLight,
    light.LightType.BRIGHTNESS_ONLY,
    icon="mdi:dots-grid",
    default_restore_mode="ALWAYS_ON",
).extend(
    {
        cv.GenerateID(CONF_WUASTATION_ID): cv.use_id(WuaStation),
        cv.Optional(
            CONF_DEFAULT_TRANSITION_LENGTH, default="0s"
        ): cv.positive_time_period_milliseconds,
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_WUASTATION_ID])
    output = cg.new_Pvariable(config[CONF_OUTPUT_ID])
    await light.register_light(output, config)
    cg.add(output.set_parent(parent))
    cg.add(parent.set_display_light(output))
