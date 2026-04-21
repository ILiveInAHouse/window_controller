import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.components import number
from esphome.const import CONF_ID

CODEOWNERS = ["@ILiveInAHouse"]
DEPENDENCIES = []
AUTO_LOAD = [ ]
MULTI_CONF = False

from esphome.components.window_controller import WindowControllerHub, window_controller_ns
WindowPositionNumber = window_controller_ns.class_("WindowPositionNumber", number.Number)

WhichMotor = window_controller_ns.enum("WhichMotorEnum")
MOTOR_ENUMS = {
    "MOTOR_NONE": WhichMotor.MOTOR_NONE,
    "MOTOR_A": WhichMotor.MOTOR_A,
    "MOTOR_B": WhichMotor.MOTOR_B,
}

# Key for the parent ID in YAML
CONF_WINDOW_CONTROLLER_ID = "window_controller_id"
CONF_WHICH_MOTOR = "which_motor"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(CONF_WINDOW_CONTROLLER_ID): cv.use_id(WindowControllerHub),
    # cv.Required(CONF_WHICH_MOTOR): cv.enum(MOTOR_ENUMS, upper=True, space="_"),
    cv.Required("speed_slider"): number.number_schema(WindowPositionNumber).extend({
        cv.Required(CONF_WHICH_MOTOR): cv.enum(MOTOR_ENUMS, upper=True, space="_"),
    }),
    cv.Required("calibration_slider"): number.number_schema(WindowPositionNumber).extend({
        cv.Required(CONF_WHICH_MOTOR): cv.enum(MOTOR_ENUMS, upper=True, space="_"),
    }),
})

async def to_code(config):
    # Find the parent component
    parent = await cg.get_variable(config[CONF_WINDOW_CONTROLLER_ID])
    if speed_slider_conf := config.get("speed_slider"):
        # Create the C++ Number object
        var = cg.new_Pvariable(speed_slider_conf[CONF_ID])
        await cg.register_component(var, speed_slider_conf)
        await number.register_number(var, speed_slider_conf, min_value=0, max_value=100, step=1)
        
        # 3. Call the C++ setter function
        cg.add(parent.set_speed_slider(var))
    if calibration_slider_conf := config.get("calibration_slider"):
        # Create the C++ Number object
        var = cg.new_Pvariable(speed_slider_conf[CONF_ID])
        await cg.register_component(var, calibration_slider_conf)
        await number.register_number(var, calibration_slider_conf, min_value=0, max_value=100, step=1)
        
        # 3. Call the C++ setter function
        cg.add(parent.set_calibration_slider(var))
