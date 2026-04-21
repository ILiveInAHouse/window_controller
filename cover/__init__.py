import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    ENTITY_CATEGORY_DIAGNOSTIC,
    STATE_CLASS_MEASUREMENT,
)

from .. import WINDOWCONTROLLER_CLIENT_SCHEMA, window_controller_ns, register_windowcontroller_child

AUTO_LOAD = []
DEPENDENCIES = ["window_controller"]

CONF_WINDOW_NUMBER = "window_number"
CONF_FAULTS = "faults"
CONF_WHICH_MOTOR = "which_motor"

WindowMotorClass = window_controller_ns.class_(
    "WindowMotorClass", cg.PollingComponent
)

WhichMotor = window_controller_ns.enum("WhichMotorEnum")
MOTOR_ENUMS = {
    "MOTOR_NONE": WhichMotor.MOTOR_NONE,
    "MOTOR_A": WhichMotor.MOTOR_A,
    "MOTOR_B": WhichMotor.MOTOR_B,
}

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(WindowMotorClass),
        cv.Required(CONF_WHICH_MOTOR): cv.enum(MOTOR_ENUMS, upper=True, space="_"),
    }
#    .extend(cv.polling_component_schema("5s"))
) .extend(WINDOWCONTROLLER_CLIENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await register_windowcontroller_child(var, config)
    cg.add(var.set_whichMotor(config[CONF_WHICH_MOTOR]))
