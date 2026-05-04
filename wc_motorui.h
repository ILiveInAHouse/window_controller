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
   WCNumber *target_position;
   WCNumber *max_torque;
   WCSensor *window_number;
   WCSensor *faults;
   WhichMotorEnum whichMotor{MOTOR_NONE};
   uint8_t boardId;

protected:
};

// TODO: Use this to link WindowMotorClasses to the parent WindowControllerHub
// TODO: At setup maybe pass callback functions to Hub
// TODO: Probably call a WindowMotorClass::linkMotor() for each motor from Hub
// TODO:   need to set up a parent pointer link in WindowMotorClass?
} // namespace