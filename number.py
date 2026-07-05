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
    "MOTOR_NUM" : WhichMotor.MOTOR_NUM,
    "MOTOR_NONE": WhichMotor.MOTOR_NONE,
    "MOTOR_A": WhichMotor.MOTOR_A,
    "MOTOR_B": WhichMotor.MOTOR_B,
}

# Key for the parent ID in YAML
CONF_WINDOW_CONTROLLER_ID = "window_controller_id"
CONF_WHICH_MOTOR = "which_motor"
CONF_TARGET_POSITION = "target_position"
CONF_MAX_CURRENT = "max_current"
CONF_OPEN_MAX_CURRENT = "open_max_current"
CONF_CLOSE_MAX_CURRENT = "close_max_current"
CONF_START_MAX_CURRENT = "start_max_current"
CONF_ALL_MOTOR_STATUS = "all_motor_status"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(CONF_WINDOW_CONTROLLER_ID): cv.use_id(WindowControllerHub),
    cv.Required(CONF_TARGET_POSITION): number.number_schema(WCNumber).extend({
        cv.Required(CONF_WHICH_MOTOR): cv.enum(MOTOR_ENUMS, upper=True, space="_"),
    }),
    cv.Required(CONF_MAX_CURRENT): number.number_schema(WCNumber).extend({
        cv.Required(CONF_WHICH_MOTOR): cv.enum(MOTOR_ENUMS, upper=True, space="_"),
    }),
    cv.Required(CONF_OPEN_MAX_CURRENT): number.number_schema(WCNumber).extend({
        cv.Required(CONF_WHICH_MOTOR): cv.enum(MOTOR_ENUMS, upper=True, space="_"),
    }),
    cv.Required(CONF_CLOSE_MAX_CURRENT): number.number_schema(WCNumber).extend({
        cv.Required(CONF_WHICH_MOTOR): cv.enum(MOTOR_ENUMS, upper=True, space="_"),
    }),
    cv.Required(CONF_START_MAX_CURRENT): number.number_schema(WCNumber).extend({
        cv.Required(CONF_WHICH_MOTOR): cv.enum(MOTOR_ENUMS, upper=True, space="_"),
    }),
    cv.Required(CONF_ALL_MOTOR_STATUS): number.number_schema(WCNumber).extend({
        cv.Required(CONF_WHICH_MOTOR): cv.enum(MOTOR_ENUMS, upper=True, space="_"),
    }),
})

async def to_code(config):
    # Find the parent component
    parent = await cg.get_variable(config[CONF_WINDOW_CONTROLLER_ID])
    if target_position_conf := config.get(CONF_TARGET_POSITION):
        var = await number.new_number(target_position_conf, min_value=0, max_value=100, step=1)
        cg.add(var.set_which_motor(target_position_conf[CONF_WHICH_MOTOR]))
        cg.add(parent.set_target_position(var))
    if max_current_conf := config.get(CONF_MAX_CURRENT):
        var = await number.new_number(max_current_conf, min_value=0, max_value=2, step=0.1)
        cg.add(var.set_which_motor(max_current_conf[CONF_WHICH_MOTOR]))
        cg.add(parent.set_max_current(var))
    if open_max_current_conf := config.get(CONF_OPEN_MAX_CURRENT):
        var = await number.new_number(open_max_current_conf, min_value=0, max_value=2, step=0.1)
        cg.add(var.set_which_motor(open_max_current_conf[CONF_WHICH_MOTOR]))
        cg.add(parent.set_open_max_current(var))
    if close_max_current_conf := config.get(CONF_CLOSE_MAX_CURRENT):
        var = await number.new_number(close_max_current_conf, min_value=0, max_value=2, step=0.1)
        cg.add(var.set_which_motor(close_max_current_conf[CONF_WHICH_MOTOR]))
        cg.add(parent.set_close_max_current(var))
    if start_max_current_conf := config.get(CONF_START_MAX_CURRENT):
        var = await number.new_number(start_max_current_conf, min_value=0, max_value=2, step=0.1)
        cg.add(var.set_which_motor(start_max_current_conf[CONF_WHICH_MOTOR]))
        cg.add(parent.set_start_max_current(var))
    if all_motor_status_conf := config.get(CONF_ALL_MOTOR_STATUS):
        var = await number.new_number(all_motor_status_conf, min_value=0, max_value=1024, step=1)
        cg.add(var.set_which_motor(max_current_conf[CONF_WHICH_MOTOR]))
        cg.add(parent.set_all_motor_status(var))
       
