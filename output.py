import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.components import output
from esphome import pins
from esphome.const import (CONF_ID, CONF_FREQUENCY, CONF_PIN)
# copied from sensor.py
CODEOWNERS = ["@ILiveInAHouse"]
DEPENDENCIES = []
AUTO_LOAD = [ ]
MULTI_CONF = False

from esphome.components.window_controller import WindowControllerHub, window_controller_ns
WCPWM = window_controller_ns.class_("WCPWM", output.FloatOutput)

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
CONF_PWM = "pwm"

# modeled off of libretiny_pwm
CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(CONF_WINDOW_CONTROLLER_ID): cv.use_id(WindowControllerHub),
    cv.Required(CONF_WHICH_MOTOR): cv.enum(MOTOR_ENUMS, upper=True, space="_"),
    cv.Required(CONF_PWM): output.FLOAT_OUTPUT_SCHEMA.extend({
        cv.Required(CONF_ID): cv.declare_id(WCPWM),
        cv.Required(CONF_PIN): pins.internal_gpio_output_pin_schema,
        # cv.Optional(CONF_FREQUENCY, default="1kHz"): cv.All(
        #     cv.frequency, cv.float_range(min=0, min_included=False)
        # ),
    }),
})

async def to_code(config):
    parent = await cg.get_variable(config[CONF_WINDOW_CONTROLLER_ID])
    if pwm_conf := config.get(CONF_PWM):
        gpio = await cg.gpio_pin_expression(pwm_conf[CONF_PIN])
        var = cg.new_Pvariable(pwm_conf[CONF_ID], gpio)
        # await cg.register_component(var, config)
        await output.register_output(var, pwm_conf)
        cg.add(var.set_which_motor(config[CONF_WHICH_MOTOR]))
        cg.add(parent.set_pwm(var))
       
