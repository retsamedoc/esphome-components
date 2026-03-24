"""ESPHome codegen entrypoint for the Roomba component.

This module defines the root ``roomba:`` configuration schema and creates the
runtime C++ component instance that owns UART communication and parsing.
"""

from esphome import automation, pins
import esphome.codegen as cg
from esphome.components import time, uart
import esphome.config_validation as cv
from esphome.const import CONF_ID

DEPENDENCIES = ["uart"]
AUTO_LOAD = ["sensor", "binary_sensor", "text_sensor", "time"]

roomba_ns = cg.esphome_ns.namespace("roomba")
Roomba = roomba_ns.class_("Roomba", cg.Component, uart.UARTDevice)

CONF_USE_STREAM = "use_stream"
CONF_AUTO_RECONNECT = "auto_reconnect"
CONF_RESTORE_STATE = "restore_state"
CONF_BRC_PIN = "brc_pin"
CONF_TIME_ID = "time_id"

SetDayTimeAction = roomba_ns.class_("SetDayTimeAction", automation.Action)
StartCleanAction = roomba_ns.class_("StartCleanAction", automation.Action)
DockAction = roomba_ns.class_("DockAction", automation.Action)
StopAction = roomba_ns.class_("StopAction", automation.Action)
ResetAction = roomba_ns.class_("ResetAction", automation.Action)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(Roomba),
        cv.Optional(CONF_USE_STREAM, default=True): cv.boolean,
        cv.Optional(CONF_AUTO_RECONNECT, default=True): cv.boolean,
        cv.Optional(CONF_RESTORE_STATE, default=False): cv.boolean,
        cv.Optional(CONF_BRC_PIN): pins.gpio_output_pin_schema,
        cv.Optional(CONF_TIME_ID): cv.use_id(time.RealTimeClock),
    }
).extend(uart.UART_DEVICE_SCHEMA)


async def to_code(config):
    """Generate C++ objects and apply user configuration.

    Also defines ``ROOMBA_MAX_OUTPUTS`` from known Python entity maps so the
    C++ fixed-size output registry is sized for the declared integration points.
    """
    from .binary_sensor import BINARY_MAP
    from .sensor import SENSOR_MAP
    from .text_sensor import TEXT_MAP

    roomba_max_outputs = len(SENSOR_MAP) + len(BINARY_MAP) + len(TEXT_MAP)
    cg.add_define("ROOMBA_MAX_OUTPUTS", roomba_max_outputs)

    cg.add_global(roomba_ns.using)
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    await uart.register_uart_device(var, config)

    cg.add(var.set_use_stream(config[CONF_USE_STREAM]))
    cg.add(var.set_auto_reconnect(config[CONF_AUTO_RECONNECT]))
    cg.add(var.set_restore_state(config[CONF_RESTORE_STATE]))

    if CONF_BRC_PIN in config:
        pin = await cg.gpio_pin_expression(config[CONF_BRC_PIN])
        cg.add(var.set_brc_pin(pin))

    if CONF_TIME_ID in config:
        time_source = await cg.get_variable(config[CONF_TIME_ID])
        cg.add(var.set_time_source(time_source))


@automation.register_action(
    "roomba.set_day_time",
    SetDayTimeAction,
    cv.Schema(
        {
            cv.Required(CONF_ID): cv.use_id(Roomba),
        }
    ),
    synchronous=True,
)
async def roomba_set_day_time_action_to_code(config, action_id, template_arg, args):
    """Build automation action for one-shot Roomba day/time sync."""
    parent = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, parent)


@automation.register_action(
    "roomba.start_clean",
    StartCleanAction,
    cv.Schema(
        {
            cv.Required(CONF_ID): cv.use_id(Roomba),
        }
    ),
    synchronous=True,
)
async def roomba_start_clean_action_to_code(config, action_id, template_arg, args):
    """Build automation action for Roomba CLEAN."""
    parent = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, parent)


@automation.register_action(
    "roomba.dock",
    DockAction,
    cv.Schema(
        {
            cv.Required(CONF_ID): cv.use_id(Roomba),
        }
    ),
    synchronous=True,
)
async def roomba_dock_action_to_code(config, action_id, template_arg, args):
    """Build automation action for Roomba SEEK_DOCK."""
    parent = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, parent)


@automation.register_action(
    "roomba.stop",
    StopAction,
    cv.Schema(
        {
            cv.Required(CONF_ID): cv.use_id(Roomba),
        }
    ),
    synchronous=True,
)
async def roomba_stop_action_to_code(config, action_id, template_arg, args):
    """Build automation action for Roomba STOP."""
    parent = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, parent)


@automation.register_action(
    "roomba.reset",
    ResetAction,
    cv.Schema(
        {
            cv.Required(CONF_ID): cv.use_id(Roomba),
        }
    ),
    synchronous=True,
)
async def roomba_reset_action_to_code(config, action_id, template_arg, args):
    """Build automation action for Roomba RESET."""
    parent = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, parent)
