#pragma once

#include "wc_number.h"
#include "wc_sensor.h"
#include "wc_whichmotor.h"
#include "wc_pwm.h"

// Namespace definition
namespace esphome::window_controller {

// Store pointers to all the UI controls and sensors that the WindowMotorClass uses.
// Also put config info from the hub for the WindowMotorClasses to use.
class WCMotorUI {
public:
   WCNumber *target_position_Number{nullptr};
   WCNumber *max_torque_Number{nullptr};
   WCNumber *all_motor_status_Number{nullptr}; // input from Hass
   WCSensor *window_number_Sensor{nullptr};
   WCSensor *faults_Sensor{nullptr};
   WCSensor *motor_status_Sensor{nullptr};  // statusMask bit is set if this window has work to do
   WCSensor *est_position_Sensor{nullptr};
   WCSensor *uptime_Sensor{nullptr};
   output::FloatOutput *pwm_FloatOutput{nullptr};
   uint8_t boardId;
   float numRotationsToFullOpen;
   float currentRotationIndex;
   float maxTorqueSeen;
   float current;
   float rpm;
   WhichMotorEnum getWhichMotor() {return this->whichMotor;}
   void setWhichMotor(WhichMotorEnum w) {this->whichMotor = w;}

protected:
   WhichMotorEnum whichMotor{MOTOR_NONE};
};

// TODO: Use this to link WindowMotorClasses to the parent WindowControllerHub
// TODO: At setup maybe pass callback functions to Hub
// TODO: Probably call a WindowMotorClass::linkMotor() for each motor from Hub
// TODO:   need to set up a parent pointer link in WindowMotorClass?
} // namespace