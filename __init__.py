import esphome.config_validation as cv
from esphome import pins
#from esphome.components import ina219
from esphome.components import i2c
import esphome.codegen as cg
from esphome.const import (
    CONF_ID,
    CONF_NUMBER
)

CODEOWNERS = ["@ILiveInAHouse"]
DEPENDENCIES = ["i2c"]
AUTO_LOAD = [ ]
MULTI_CONF = True

CONF_WINDOWCONTROLLER_ID = "window_controller_id"

# Define constants for configuration keys
CONF_BOARDID0_PIN = "boardid0_pin"
CONF_BOARDID1_PIN = "boardid1_pin"
CONF_BOARDID2_PIN = "boardid2_pin"

CONF_MOTAENCA_PIN = "mota_enca_pin"
CONF_MOTAENCB_PIN = "mota_encb_pin"
CONF_MOTAPWM_PIN = "mota_pwm_pin"
CONF_MOTAIN1_PIN = "mota_in1_pin"
CONF_MOTAIN2_PIN = "mota_in2_pin"

CONF_MOTBENCA_PIN = "motb_enca_pin"
CONF_MOTBENCB_PIN = "motb_encb_pin"
CONF_MOTBPWM_PIN = "motb_pwm_pin"
CONF_MOTBIN1_PIN = "motb_in1_pin"
CONF_MOTBIN2_PIN = "motb_in2_pin"

# C++ namespace
window_controller_ns = cg.esphome_ns.namespace("window_controller")
# Create class and inherit from
# also look at uart.UARTDevice as an example
WindowControllerHub = window_controller_ns.class_("WindowControllerHub", cg.PollingComponent, i2c.I2CDevice)

# Look at Parent Hub and Child components
# Look at cv.use_id
# Use bedjet as example of parent hub and children with multiple sensors

# Define configuration keys for two different addresses
CONF_MOTA_INA219_ADDRESS = "mota_ina219_address"
CONF_MOTB_INA219_ADDRESS = "motb_ina219_address"

CONFIG_SCHEMA = (
    cv.COMPONENT_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(WindowControllerHub),
            # Schema definition, containing the options available for the component
            cv.Required(CONF_MOTA_INA219_ADDRESS): cv.i2c_address,
            cv.Required(CONF_MOTB_INA219_ADDRESS): cv.i2c_address,
            cv.Required(CONF_BOARDID0_PIN): pins.gpio_input_pin_schema,
            cv.Required(CONF_BOARDID1_PIN): pins.gpio_input_pin_schema,
            cv.Required(CONF_BOARDID2_PIN): pins.gpio_input_pin_schema,

            cv.Required(CONF_MOTAENCA_PIN): pins.gpio_input_pin_schema,
            cv.Required(CONF_MOTAENCB_PIN): pins.gpio_input_pin_schema,
            cv.Required(CONF_MOTAPWM_PIN): pins.gpio_input_pin_schema,
            cv.Required(CONF_MOTAIN1_PIN): pins.gpio_input_pin_schema,
            cv.Required(CONF_MOTAIN2_PIN): pins.gpio_input_pin_schema,

            cv.Required(CONF_MOTBENCA_PIN): pins.gpio_input_pin_schema,
            cv.Required(CONF_MOTBENCB_PIN): pins.gpio_input_pin_schema,
            cv.Required(CONF_MOTBPWM_PIN): pins.gpio_input_pin_schema,
            cv.Required(CONF_MOTBIN1_PIN): pins.gpio_input_pin_schema,
            cv.Required(CONF_MOTBIN2_PIN): pins.gpio_input_pin_schema,
            # Optional example
            # cv.Optional(CONF_BAZ): cv.int_range(0, 255),
        }
    )
    .extend(cv.polling_component_schema("5s"))
    #.extend(cv.COMPONENT_SCHEMA).extend(i2c.i2c_device_schema(0x40)) # Default I2C address
    .extend(i2c.i2c_device_schema(None))
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
    # Set the addresses for the two internal I2CDevice objects
    cg.add(var.motA.ina219.set_i2c_address(config[CONF_MOTA_INA219_ADDRESS]))
    cg.add(var.motB.ina219.set_i2c_address(config[CONF_MOTB_INA219_ADDRESS]))
    
    # Important: set the parent bus for both
    parent = await cg.get_variable(config[i2c.CONF_I2C_ID])
    cg.add(var.motA.ina219.set_i2c_bus(parent))
    cg.add(var.motB.ina219.set_i2c_bus(parent))
    
    pin = await cg.gpio_pin_expression(config[CONF_BOARDID0_PIN])
    cg.add(var.set_boardid0_pin(pin))
    pin = await cg.gpio_pin_expression(config[CONF_BOARDID1_PIN])
    cg.add(var.set_boardid1_pin(pin))
    pin = await cg.gpio_pin_expression(config[CONF_BOARDID2_PIN])
    cg.add(var.set_boardid2_pin(pin))

    pin = await cg.gpio_pin_expression(config[CONF_MOTAENCA_PIN])
    cg.add(var.set_mota_enca_pin(pin))
    pin = await cg.gpio_pin_expression(config[CONF_MOTAENCB_PIN])
    cg.add(var.set_mota_encb_pin(pin))
    pin = await cg.gpio_pin_expression(config[CONF_MOTAPWM_PIN])
    cg.add(var.set_mota_pwm_pin(pin))
    pin = await cg.gpio_pin_expression(config[CONF_MOTAIN1_PIN])
    cg.add(var.set_mota_in1_pin(pin))
    pin = await cg.gpio_pin_expression(config[CONF_MOTAIN2_PIN])
    cg.add(var.set_mota_in2_pin(pin))

    pin = await cg.gpio_pin_expression(config[CONF_MOTBENCA_PIN])
    cg.add(var.set_motb_enca_pin(pin))
    pin = await cg.gpio_pin_expression(config[CONF_MOTBENCB_PIN])
    cg.add(var.set_motb_encb_pin(pin))
    pin = await cg.gpio_pin_expression(config[CONF_MOTBPWM_PIN])
    cg.add(var.set_motb_pwm_pin(pin))
    pin = await cg.gpio_pin_expression(config[CONF_MOTBIN1_PIN])
    cg.add(var.set_motb_in1_pin(pin))
    pin = await cg.gpio_pin_expression(config[CONF_MOTBIN2_PIN])
    cg.add(var.set_motb_in2_pin(pin))
# Configure the component