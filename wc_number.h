#pragma once

#include "esphome/components/number/number.h"
#include "wc_whichmotor.h"

// Namespace definition
namespace esphome::window_controller {

// Forward declare WindowControllerHub
class WindowControllerHub;

// Create a non-abstract number class
class WCNumber : public number::Number {
public:
  // Store a pointer to the parent hub
  void set_parent(WindowControllerHub *parent) { this->parent_ = parent; }
  void set_which_motor(WhichMotorEnum which) {this->whichMotor = which; }
  WhichMotorEnum whichMotor{MOTOR_NONE};
protected:
  WindowControllerHub *parent_;
  void control(float value) override;
};

} // namespace