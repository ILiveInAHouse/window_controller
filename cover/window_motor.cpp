#include "window_motor.h"

#include "esphome/core/log.h"

namespace esphome {
namespace window_controller {

   static const char *const TAG = "windowmotor";

void WindowMotorClass::child_setup() {
}

void WindowMotorClass::update() {
}

void WindowMotorClass::child_publish_info() {
}

void WindowMotorClass::child_update() {
   ESP_LOGI(TAG, "motor=%c child_update", (this->whichMotor == MOTOR_A) ? 'A' : 'B');
}
}
}
