import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.components import sensor
from esphome.const import (
    CONF_ID,
    ENTITY_CATEGORY_DIAGNOSTIC,
    STATE_CLASS_MEASUREMENT,
)

CODEOWNERS = ["@ILiveInAHouse"]
DEPENDENCIES = []
AUTO_LOAD = [ ]
MULTI_CONF = False

from esphome.components.window_controller import WindowControllerHub, window_controller_ns
WCSensor = window_controller_ns.class_("WCSensor", sensor.Sensor)

WhichMotor = window_controller_ns.enum("WhichMotorEnum")
MOTOR_ENUMS = {
    "MOTOR_NONE": WhichMotor.MOTOR_NONE,
    "MOTOR_A": WhichMotor.MOTOR_A,
    "MOTOR_B": WhichMotor.MOTOR_B,
}

# Key for the parent ID in YAML
CONF_WINDOW_CONTROLLER_ID = "window_controller_id"
CONF_WHICH_MOTOR = "which_motor"
CONF_WINDOW_NUMBER = "window_number"
CONF_FAULTS = "faults"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(CONF_WINDOW_CONTROLLER_ID): cv.use_id(WindowControllerHub),
    cv.Required(CONF_WHICH_MOTOR): cv.enum(MOTOR_ENUMS, upper=True, space="_"),
    cv.Optional(CONF_WINDOW_NUMBER): sensor.sensor_schema(
        state_class=STATE_CLASS_MEASUREMENT,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
    ),
    cv.Optional(CONF_FAULTS): sensor.sensor_schema(
        state_class=STATE_CLASS_MEASUREMENT,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
    ),
})

async def to_code(config):
    parent = await cg.get_variable(config[CONF_WINDOW_CONTROLLER_ID])
    if window_number_conf := config.get(CONF_WINDOW_NUMBER):
        var = await sensor.new_sensor(window_number_conf)
        cg.add(var.set_which_motor(config[CONF_WHICH_MOTOR]))
        cg.add(parent.set_window_number(var))
    if faults_conf := config.get(CONF_FAULTS):
        var = await sensor.new_sensor(faults_conf)
        cg.add(var.set_which_motor(config[CONF_WHICH_MOTOR]))
        cg.add(parent.set_faults(var))
       
