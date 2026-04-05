import esphome.config_validation as cv
from esphome import pins
import esphome.codegen as cg
from esphome.const import (
    CONF_ID,
)

CODEOWNERS = ["@ILiveInAHouse"]
DEPENDENCIES = [ ]
AUTO_LOAD = [ ]
MULTI_CONF = True

# Define constants for configuration keys
CONF_BOARDID0_PIN = "boardid0_pin"
CONF_BOARDID1_PIN = "boardid1_pin"
CONF_BOARDID2_PIN = "boardid2_pin"

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
    # Configure the component