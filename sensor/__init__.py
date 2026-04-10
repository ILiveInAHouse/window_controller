import esphome.codegen as cg
from esphome.components import sensor
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
)

# modeled off of ads1115
from .. import CONF_WINDOWCONTROLLER_ID, WindowController, winctrl_ns

AUTO_LOAD = []
DEPENDENCIES = ["windowController"]

WindowControllerSensor = winctrl_ns.class_(
    "WindowControllerSensor", sensor.Sensor, cg.PollingComponent
)

CONFIG_SCHEMA = (
    sensor.sensor_schema(
        WindowControllerSensor,
    )
    .extend(
        {
            cv.GenerateID(CONF_WINDOWCONTROLLER_ID): cv.use_id(WindowController),
        }
    )
    .extend(cv.polling_component_schema("5s"))
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await sensor.register_sensor(var, config)
    await cg.register_component(var, config)
    await cg.register_parented(var, config[CONF_WINDOWCONTROLLER_ID])
