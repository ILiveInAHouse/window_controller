#pragma once

// Namespace definition
namespace esphome::window_controller {

enum WhichMotorEnum { MOTOR_A = 0, MOTOR_B = 1, MOTOR_NUM = 2, MOTOR_NONE=4 };

inline bool whichMotorIsValid(WhichMotorEnum w) {
   if ((w == MOTOR_A) || (w == MOTOR_B)) {
      return true;
   } else {
      return false;
   }
}

} // namespace