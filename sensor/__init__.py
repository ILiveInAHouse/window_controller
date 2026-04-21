import esphome.codegen as cg
from esphome.components import sensor
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    ENTITY_CATEGORY_DIAGNOSTIC,
    STATE_CLASS_MEASUREMENT,
)

# modeled off of ads1115 which has only 1 sensor value
#from .. import CONF_WINDOWCONTROLLER_ID, WindowController, window_controller_ns
from .. import WINDOWCONTROLLER_CLIENT_SCHEMA, window_controller_ns, register_windowcontroller_child
# bedjet component has multiple sensor values
#   .yaml document: https://esphome.io/components/climate/bedjet/#bedjet-sensor

AUTO_LOAD = []
DEPENDENCIES = ["window_controller"]

CONF_WINDOW_NUMBER = "window_number"
CONF_FAULTS = "faults"
CONF_WHICH_MOTOR = "which_motor"

WindowControllerSensor = window_controller_ns.class_(
    "WindowControllerSensor", cg.PollingComponent
)

WhichMotor = window_controller_ns.enum("WhichMotorEnum")
MOTOR_ENUMS = {
    "MOTOR_NONE": WhichMotor.MOTOR_NONE,
    "MOTOR_A": WhichMotor.MOTOR_A,
    "MOTOR_B": WhichMotor.MOTOR_B,
}

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(WindowControllerSensor),
        cv.Required(CONF_WHICH_MOTOR): cv.enum(MOTOR_ENUMS, upper=True, space="_"),
        cv.Optional(CONF_WINDOW_NUMBER): sensor.sensor_schema(
            state_class=STATE_CLASS_MEASUREMENT,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        cv.Optional(CONF_FAULTS): sensor.sensor_schema(
            state_class=STATE_CLASS_MEASUREMENT,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
    }
#    .extend(cv.polling_component_schema("5s"))
) .extend(WINDOWCONTROLLER_CLIENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await register_windowcontroller_child(var, config)

    if window_number_sensor := config.get(CONF_WINDOW_NUMBER):
        sensor_var = await sensor.new_sensor(window_number_sensor)
        cg.add(var.set_window_number_sensor(sensor_var))

    if faults_sensor := config.get(CONF_FAULTS):
        sensor_var = await sensor.new_sensor(faults_sensor)
        cg.add(var.set_faults_sensor(sensor_var))
    cg.add(var.set_whichMotor(config[CONF_WHICH_MOTOR]))

