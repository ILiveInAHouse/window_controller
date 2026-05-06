#pragma once

#include "esphome/components/output/float_output.h"
#include "wc_whichmotor.h"

// Namespace definition
namespace esphome::window_controller {

// Forward declare WindowControllerHub
class WindowControllerHub;

// Create a non-abstract sensor class
class WCPWM : public output::FloatOutput {
public:
  explicit WCPWM(InternalGPIOPin *pin) : pin_(pin) {}
  // Store a pointer to the parent hub
  void set_parent(WindowControllerHub *parent) { this->parent_ = parent; }
  void set_which_motor(WhichMotorEnum which) {this->whichMotor = which; }
  WhichMotorEnum whichMotor{MOTOR_NONE};
  void write_state(float state) override {this->set_level(state);}
protected:
  WindowControllerHub *parent_;
  InternalGPIOPin *pin_;
  // void control(float value) override;
};

} // namespace