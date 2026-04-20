import esphome.codegen as cg
from esphome.components import number
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    UNIT_PERCENT,
)

from .. import CONF_WINDOWCONTROLLER_ID, WINDOWCONTROLLER_CLIENT_SCHEMA, winctrl_ns, register_windowcontroller_child
# bedjet component has multiple sensor values
#   .yaml document: https://esphome.io/components/climate/bedjet/#bedjet-sensor

AUTO_LOAD = []
DEPENDENCIES = ["window_controller"]

CONF_WHICH_MOTOR = "which_motor"
CONF_TARGET_POSITION = "target_position"

WindowControllerNumber = winctrl_ns.class_(
    "WindowControllerNumber", cg.PollingComponent
)

WhichMotor = winctrl_ns.enum("WhichMotorEnum")
MOTOR_ENUMS = {
    "MOTOR_NONE": WhichMotor.MOTOR_NONE,
    "MOTOR_A": WhichMotor.MOTOR_A,
    "MOTOR_B": WhichMotor.MOTOR_B,
}

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(WindowControllerNumber),
        cv.Required(CONF_WHICH_MOTOR): cv.enum(MOTOR_ENUMS, upper=True, space="_"),
        # starting with cover component as example for position, but it's
        #   templatable.  
        # cv.Optional(CONF_TARGET_POSITION): cv.float_range(min=0.0, max=1.0),
        cv.Optional(CONF_TARGET_POSITION): number.number_schema(
            WindowControllerNumber,
            unit_of_measurement=UNIT_PERCENT,
        ),
    }
#    .extend(cv.polling_component_schema("5s"))
) .extend(WINDOWCONTROLLER_CLIENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await register_windowcontroller_child(var, config)

    cg.add(var.set_whichMotor(config[CONF_WHICH_MOTOR]))
    if target_position_number := config.get(CONF_TARGET_POSITION):
        number_var = await number.new_number(target_position_number,
                                             min_value=0.0,
                                             max_value=100.0,
                                             step=1)
        await cg.register_parented(number_var, config[CONF_WINDOWCONTROLLER_ID])
        cg.add(var.set_target_position_number(number_var))
    # await number.register_number(
    #     var, 
    #     config, 
    #     min_value=0, 
    #     max_value=100, 
    #     step=1
    # )

