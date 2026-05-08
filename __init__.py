import esphome.config_validation as cv
from esphome import pins
from esphome.components import output, rotary_encoder
import esphome.codegen as cg
from esphome.const import (
    CONF_ID,
    CONF_NUMBER
)

CODEOWNERS = ["@ILiveInAHouse"]
DEPENDENCIES = []
AUTO_LOAD = [ ]

CONF_WINDOWCONTROLLER_ID = "window_controller_id"

# Define constants for configuration keys
CONF_BOARDID0_PIN = "boardid0_pin"
CONF_BOARDID1_PIN = "boardid1_pin"
CONF_BOARDID2_PIN = "boardid2_pin"
CONF_MOTOR_A_PWM = "motor_a_pwm"
CONF_MOTOR_B_PWM = "motor_b_pwm"
CONF_MOTOR_A_ENC = "motor_a_enc"
CONF_MOTOR_B_ENC = "motor_b_enc"

# C++ namespace
window_controller_ns = cg.esphome_ns.namespace("window_controller")
# Create class and inherit from
# also look at uart.UARTDevice as an example
WindowControllerHub = window_controller_ns.class_("WindowControllerHub", cg.PollingComponent)

# Look at Parent Hub and Child components
# Look at cv.use_id
# Use bedjet as example of parent hub and children with multiple sensors

CONFIG_SCHEMA = (
    cv.COMPONENT_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(WindowControllerHub),
            # Schema definition, containing the options available for the component
            cv.Required(CONF_BOARDID0_PIN): pins.gpio_input_pin_schema,
            cv.Required(CONF_BOARDID1_PIN): pins.gpio_input_pin_schema,
            cv.Required(CONF_BOARDID2_PIN): pins.gpio_input_pin_schema,
            cv.Required(CONF_MOTOR_A_PWM): cv.use_id(output.FloatOutput),
            cv.Required(CONF_MOTOR_B_PWM): cv.use_id(output.FloatOutput),
            cv.Required(CONF_MOTOR_A_ENC): cv.use_id(rotary_encoder),
            cv.Required(CONF_MOTOR_B_ENC): cv.use_id(rotary_encoder),
            # Optional example
            # cv.Optional(CONF_BAZ): cv.int_range(0, 255),
        }
    )
    .extend(cv.polling_component_schema("5s"))
    #.extend(cv.COMPONENT_SCHEMA).extend(i2c.i2c_device_schema(0x40)) # Default I2C address
    # .extend(i2c.i2c_device_schema(None))
)

WINDOWCONTROLLER_CLIENT_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_WINDOWCONTROLLER_ID): cv.use_id(WindowControllerHub),
    }
)

async def register_windowcontroller_child(var, config):
    parent = await cg.get_variable(config[CONF_WINDOWCONTROLLER_ID])
    cg.add(parent.register_child(var))

async def to_code(config):
    # Declare new component
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    pin = await cg.gpio_pin_expression(config[CONF_BOARDID0_PIN])
    cg.add(var.set_boardid0_pin(pin))
    pin = await cg.gpio_pin_expression(config[CONF_BOARDID1_PIN])
    cg.add(var.set_boardid1_pin(pin))
    pin = await cg.gpio_pin_expression(config[CONF_BOARDID2_PIN])
    cg.add(var.set_boardid2_pin(pin))
    pwm_a = await cg.get_variable(config[CONF_MOTOR_A_PWM])
    cg.add(var.set_pwm_a(pwm_a))
    pwm_b = await cg.get_variable(config[CONF_MOTOR_B_PWM])
    cg.add(var.set_pwm_b(pwm_b))
    enc_a = await cg.get_variable(config[CONF_MOTOR_A_ENC])
    cg.add(var.set_enc_a(enc_a))
    enc_b = await cg.get_variable(config[CONF_MOTOR_B_ENC])
    cg.add(var.set_enc_b(enc_b))
# Configure the component