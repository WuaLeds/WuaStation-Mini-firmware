"""Station readings for Home Assistant: indoor sensor and outdoor weather."""

import esphome.codegen as cg
from esphome.components import sensor
import esphome.config_validation as cv
from esphome.const import (
    DEVICE_CLASS_AQI,
    DEVICE_CLASS_ATMOSPHERIC_PRESSURE,
    DEVICE_CLASS_HUMIDITY,
    DEVICE_CLASS_TEMPERATURE,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
    UNIT_HECTOPASCAL,
    UNIT_PERCENT,
)

from . import CONF_WUASTATION_ID, WuaStation

DEPENDENCIES = ["wuastation"]

SENSORS = {
    "indoor_temperature": sensor.sensor_schema(
        unit_of_measurement=UNIT_CELSIUS,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_TEMPERATURE,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
    "indoor_humidity": sensor.sensor_schema(
        unit_of_measurement=UNIT_PERCENT,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_HUMIDITY,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
    "indoor_pressure": sensor.sensor_schema(
        unit_of_measurement=UNIT_HECTOPASCAL,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_ATMOSPHERIC_PRESSURE,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
    "outdoor_temperature": sensor.sensor_schema(
        unit_of_measurement=UNIT_CELSIUS,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_TEMPERATURE,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
    "outdoor_humidity": sensor.sensor_schema(
        unit_of_measurement=UNIT_PERCENT,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_HUMIDITY,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
    "rain_probability": sensor.sensor_schema(
        unit_of_measurement=UNIT_PERCENT,
        accuracy_decimals=0,
        icon="mdi:weather-rainy",
        state_class=STATE_CLASS_MEASUREMENT,
    ),
    "uv_index": sensor.sensor_schema(
        accuracy_decimals=1,
        icon="mdi:sun-wireless",
        state_class=STATE_CLASS_MEASUREMENT,
    ),
    "air_quality_index": sensor.sensor_schema(
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_AQI,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
}

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_WUASTATION_ID): cv.use_id(WuaStation),
        **{cv.Optional(key): schema for key, schema in SENSORS.items()},
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_WUASTATION_ID])
    for key in SENSORS:
        if key in config:
            sens = await sensor.new_sensor(config[key])
            cg.add(getattr(parent, f"set_{key}_sensor")(sens))
