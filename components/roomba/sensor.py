"""
Roomba sensor bindings.

Maps packet/index pairs to ESPHome sensors with full Home Assistant metadata.

Map tuple schema:
    "<yaml_key>": (<packet_id>, <index>, <sensor_schema_metadata>)
"""

import esphome.codegen as cg
from esphome.components import sensor
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_BATTERY,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_DISTANCE,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_VOLTAGE,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
    UNIT_METER,
    UNIT_MILLIAMP,
    UNIT_PERCENT,
    UNIT_VOLT,
)

from . import Roomba

SENSOR_MAP = {
    "distance": (
        19,
        0,
        {
            "unit_of_measurement": UNIT_METER,
            "device_class": DEVICE_CLASS_DISTANCE,
            "state_class": STATE_CLASS_MEASUREMENT,
            "icon": "mdi:map-marker-distance",
        },
    ),
    "angle": (
        20,
        0,
        {
            "unit_of_measurement": "°",
            "state_class": STATE_CLASS_MEASUREMENT,
            "icon": "mdi:rotate-right",
        },
    ),
    "battery": (
        26,
        0,
        {
            "unit_of_measurement": UNIT_PERCENT,
            "device_class": DEVICE_CLASS_BATTERY,
            "state_class": STATE_CLASS_MEASUREMENT,
        },
    ),
    "voltage": (
        22,
        0,
        {
            "unit_of_measurement": UNIT_VOLT,
            "device_class": DEVICE_CLASS_VOLTAGE,
            "state_class": STATE_CLASS_MEASUREMENT,
        },
    ),
    "current": (
        23,
        0,
        {
            "unit_of_measurement": UNIT_MILLIAMP,
            "device_class": DEVICE_CLASS_CURRENT,
            "state_class": STATE_CLASS_MEASUREMENT,
        },
    ),
    "temperature": (
        24,
        0,
        {
            "unit_of_measurement": UNIT_CELSIUS,
            "device_class": DEVICE_CLASS_TEMPERATURE,
            "state_class": STATE_CLASS_MEASUREMENT,
        },
    ),
}
"""Mapping of YAML sensor keys to (packet_id, index, metadata).

The packet/index pair must match the C++ decoder publish key:
``(packet_id << 8) | index``.
"""

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.use_id(Roomba),
        **{cv.Optional(name): sensor.sensor_schema(**meta) for name, (_, _, meta) in SENSOR_MAP.items()},
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    """Generate C++ registrations for configured numeric sensors."""
    parent = await cg.get_variable(config[CONF_ID])

    for name, (packet, index, _) in SENSOR_MAP.items():
        if name in config:
            sens = await sensor.new_sensor(config[name])
            cg.add(parent.register_sensor(packet, index, sens))
