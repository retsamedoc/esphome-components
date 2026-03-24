"""
Roomba text sensor bindings

Provides human-readable enum values from mapped packet/index pairs

Map tuple schema:
    "<yaml_key>": (<packet_id>, <index>, <text_sensor_schema_metadata>)
"""

import esphome.codegen as cg
from esphome.components import text_sensor
import esphome.config_validation as cv
from esphome.const import CONF_ID

from . import Roomba

TEXT_MAP = {
    "charging_state": (
        21,
        0,
        {
            "icon": "mdi:battery-charging",
        },
    ),
    "oi_mode": (
        35,
        0,
        {
            "icon": "mdi:robot-vacuum",
        },
    ),
}
"""Mapping of YAML text sensor keys to (packet_id, index, metadata)."""

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.use_id(Roomba),
        **{cv.Optional(name): text_sensor.text_sensor_schema(**meta) for name, (_, _, meta) in TEXT_MAP.items()},
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    """Generate C++ registrations for configured text sensors."""
    parent = await cg.get_variable(config[CONF_ID])
    for name, (packet, index, _) in TEXT_MAP.items():
        if name in config:
            sens = await text_sensor.new_text_sensor(config[name])
            cg.add(parent.register_text(packet, index, sens))
