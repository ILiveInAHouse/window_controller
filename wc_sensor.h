#pragma once

#include "esphome/components/sensor/sensor.h"
#include "wc_whichmotor.h"

// Namespace definition
namespace esphome::window_controller {

// Forward declare WindowControllerHub
class WindowControllerHub;

// Create a non-abstract sensor class
class WCSensor : public sensor::Sensor {
public:
  // Store a pointer to the parent hub
  void set_parent(WindowControllerHub *parent) { this->parent_ = parent; }
  void set_which_motor(WhichMotorEnum which) {this->whichMotor = which; }
protected:
  WindowControllerHub *parent_;
  // void control(float value) override;
  WhichMotorEnum whichMotor{MOTOR_NONE};
};

} // namespace