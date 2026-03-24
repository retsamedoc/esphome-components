"""
Roomba binary sensor bindings.

Maps packet/index pairs to ESPHome binary sensors.

Map tuple schema:
    "<yaml_key>": (<packet_id>, <index>, <binary_sensor_schema_metadata>)
"""

import esphome.codegen as cg
from esphome.components import binary_sensor
import esphome.config_validation as cv
from esphome.const import CONF_ID, DEVICE_CLASS_PROBLEM

from . import Roomba

BINARY_MAP = {
    "bump_right": (7, 0, {"device_class": DEVICE_CLASS_PROBLEM}),
    "bump_left": (7, 1, {"device_class": DEVICE_CLASS_PROBLEM}),
    "wheel_drop_right": (7, 2, {"device_class": DEVICE_CLASS_PROBLEM}),
    "wheel_drop_left": (7, 3, {"device_class": DEVICE_CLASS_PROBLEM}),
    "wall": (8, 0, {}),
    "cliff_left": (9, 0, {"device_class": DEVICE_CLASS_PROBLEM}),
    "cliff_front_left": (10, 0, {"device_class": DEVICE_CLASS_PROBLEM}),
    "cliff_front_right": (11, 0, {"device_class": DEVICE_CLASS_PROBLEM}),
    "cliff_right": (12, 0, {"device_class": DEVICE_CLASS_PROBLEM}),
}
"""Mapping of YAML binary keys to (packet_id, index, metadata)."""

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.use_id(Roomba),
        **{cv.Optional(name): binary_sensor.binary_sensor_schema(**meta) for name, (_, _, meta) in BINARY_MAP.items()},
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    """Generate C++ registrations for configured binary sensors."""
    parent = await cg.get_variable(config[CONF_ID])
    for name, (packet, index, _) in BINARY_MAP.items():
        if name in config:
            sens = await binary_sensor.new_binary_sensor(config[name])
            cg.add(parent.register_binary(packet, index, sens))
