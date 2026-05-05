#pragma once

#include "wc_number.h"
#include "wc_sensor.h"
#include "wc_whichmotor.h"

// Namespace definition
namespace esphome::window_controller {

// Forward declare WindowControllerHub
//class WindowControllerHub;

// Store pointers to all the UI controls and sensors that the WindowMotorClass uses.
// Also put config info from the hub for the WindowMotorClasses to use.
class WCMotorUI {
public:
   WCNumber *target_position_Number;
   WCNumber *max_torque_Number;
   WCSensor *window_number_Sensor;
   WCSensor *faults_Sensor;
   WhichMotorEnum whichMotor{MOTOR_NONE};
   uint8_t boardId;
   uint16_t status; // statusMask bit is set if this window has work to do
   uint16_t statusMask;
    uint16_t allMotorStatus;  // input from Hass
    uint32_t faults;
    float numRotationsToFullOpen;
    float currentRotationIndex;
    float maxTorqueSeen;
    float current;
    float rpm;

protected:
};

// TODO: Use this to link WindowMotorClasses to the parent WindowControllerHub
// TODO: At setup maybe pass callback functions to Hub
// TODO: Probably call a WindowMotorClass::linkMotor() for each motor from Hub
// TODO:   need to set up a parent pointer link in WindowMotorClass?
} // namespace