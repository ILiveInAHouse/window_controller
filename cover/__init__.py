import esphome.codegen as cg
from esphome import pins
from esphome.components import i2c
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
CONF_ENCA_PIN = "enca_pin"
CONF_ENCB_PIN = "encb_pin"
CONF_PWM_PIN = "pwm_pin"
CONF_IN1_PIN = "in1_pin"
CONF_IN2_PIN = "in2_pin"
CONF_INA219_ADDRESS = "ina219_address"

WindowMotorClass = window_controller_ns.class_(
    "WindowMotorClass", cg.PollingComponent
)

WhichMotor = window_controller_ns.enum("WhichMotorEnum")
MOTOR_ENUMS = {
    "MOTOR_NUM" : WhichMotor.MOTOR_NUM,
    "MOTOR_NONE": WhichMotor.MOTOR_NONE,
    "MOTOR_A": WhichMotor.MOTOR_A,
    "MOTOR_B": WhichMotor.MOTOR_B,
}

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(WindowMotorClass),
        cv.Required(CONF_WHICH_MOTOR): cv.enum(MOTOR_ENUMS, upper=True, space="_"),
        cv.Required(CONF_ENCA_PIN): pins.gpio_input_pin_schema,
        cv.Required(CONF_ENCB_PIN): pins.gpio_input_pin_schema,
        cv.Required(CONF_IN1_PIN): pins.gpio_input_pin_schema,
        cv.Required(CONF_IN2_PIN): pins.gpio_input_pin_schema,
        cv.Required(CONF_INA219_ADDRESS): cv.i2c_address,
    }
)
CONFIG_SCHEMA = CONFIG_SCHEMA.extend(cv.polling_component_schema("500ms"))
CONFIG_SCHEMA = CONFIG_SCHEMA.extend(WINDOWCONTROLLER_CLIENT_SCHEMA)
CONFIG_SCHEMA = CONFIG_SCHEMA.extend(i2c.i2c_device_schema(None))

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await register_windowcontroller_child(var, config)
    cg.add(var.setWhichMotor(config[CONF_WHICH_MOTOR]))
    pin = await cg.gpio_pin_expression(config[CONF_ENCA_PIN])
    cg.add(var.set_enca_pin(pin))
    pin = await cg.gpio_pin_expression(config[CONF_ENCB_PIN])
    cg.add(var.set_encb_pin(pin))
    # pin = await cg.gpio_pin_expression(config[CONF_PWM_PIN])
    # cg.add(var.set_pwm_pin(pin))
    pin = await cg.gpio_pin_expression(config[CONF_IN1_PIN])
    cg.add(var.set_in1_pin(pin))
    pin = await cg.gpio_pin_expression(config[CONF_IN2_PIN])
    cg.add(var.set_in2_pin(pin))
    cg.add(var.ina219.set_i2c_address(config[CONF_INA219_ADDRESS]))
    # await register_windowcontroller_i2c(var, config)
    # parent = await cg.get_variable(config[CONF_WINDOWCONTROLLER_ID])
    i2cbus = await cg.get_variable(config[i2c.CONF_I2C_ID])
    cg.add(var.ina219.set_i2c_bus(i2cbus))