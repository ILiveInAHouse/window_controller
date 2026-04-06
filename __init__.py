import esphome.config_validation as cv
from esphome import pins
import esphome.codegen as cg
from esphome.const import (
    CONF_ID,
)

CODEOWNERS = ["@ILiveInAHouse"]
DEPENDENCIES = []
AUTO_LOAD = [ ]
MULTI_CONF = True

# Define constants for configuration keys
CONF_BOARDID0_PIN = "boardid0_pin"
CONF_BOARDID1_PIN = "boardid1_pin"
CONF_BOARDID2_PIN = "boardid2_pin"
CONF_SCL_PIN = "scl_pin"
CONF_SDA_PIN = "sda_pin"

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
ns = cg.esphome_ns.namespace("window_controller")
# Create class and inherit from
WindowController = ns.class_("WindowController", cg.PollingComponent)

CONFIG_SCHEMA = cv.COMPONENT_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(WindowController),
        # Schema definition, containing the options available for the component
        cv.Required(CONF_BOARDID0_PIN): pins.gpio_input_pin_schema,
        cv.Required(CONF_BOARDID1_PIN): pins.gpio_input_pin_schema,
        cv.Required(CONF_BOARDID2_PIN): pins.gpio_input_pin_schema,
        cv.Required(CONF_SCL_PIN): pins.gpio_input_pin_schema,
        cv.Required(CONF_SDA_PIN): pins.gpio_input_pin_schema,

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
).extend(cv.polling_component_schema("5s"))

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
    pin = await cg.gpio_pin_expression(config[CONF_SCL_PIN])
    cg.add(var.set_scl_pin(pin))
    pin = await cg.gpio_pin_expression(config[CONF_SDA_PIN])
    cg.add(var.set_sda_pin(pin))

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