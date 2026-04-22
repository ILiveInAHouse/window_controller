import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.components import number
from esphome.const import CONF_ID

CODEOWNERS = ["@ILiveInAHouse"]
DEPENDENCIES = []
AUTO_LOAD = [ ]
MULTI_CONF = False

from esphome.components.window_controller import WindowControllerHub, window_controller_ns
WCNumber = window_controller_ns.class_("WCNumber", number.Number)

WhichMotor = window_controller_ns.enum("WhichMotorEnum")
MOTOR_ENUMS = {
    "MOTOR_NONE": WhichMotor.MOTOR_NONE,
    "MOTOR_A": WhichMotor.MOTOR_A,
    "MOTOR_B": WhichMotor.MOTOR_B,
}

# Key for the parent ID in YAML
CONF_WINDOW_CONTROLLER_ID = "window_controller_id"
CONF_WHICH_MOTOR = "which_motor"
CONF_TARGET_POSITION = "target_position"
CONF_MAX_TORQUE = "max_torque"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(CONF_WINDOW_CONTROLLER_ID): cv.use_id(WindowControllerHub),
    cv.Required(CONF_TARGET_POSITION): number.number_schema(WCNumber).extend({
        cv.Required(CONF_WHICH_MOTOR): cv.enum(MOTOR_ENUMS, upper=True, space="_"),
    }),
    cv.Required(CONF_MAX_TORQUE): number.number_schema(WCNumber).extend({
        cv.Required(CONF_WHICH_MOTOR): cv.enum(MOTOR_ENUMS, upper=True, space="_"),
    }),
})

async def to_code(config):
    # Find the parent component
    parent = await cg.get_variable(config[CONF_WINDOW_CONTROLLER_ID])
    if target_position_conf := config.get(CONF_TARGET_POSITION):
        var = await number.new_number(target_position_conf, min_value=0, max_value=100, step=1)
        cg.add(var.set_which_motor(config[CONF_WHICH_MOTOR]))
        cg.add(parent.set_target_position(var))
    if max_torque_conf := config.get(CONF_MAX_TORQUE):
        var = await number.new_number(max_torque_conf, min_value=0, max_value=100, step=1)
        cg.add(var.set_which_motor(config[CONF_WHICH_MOTOR]))
        cg.add(parent.set_max_torque(var))
