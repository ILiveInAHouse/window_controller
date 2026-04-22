#include "window_motor.h"

#include "esphome/core/log.h"

namespace esphome {
namespace window_controller {

static const char *const TAG = "windowmotor";

void WindowMotorClass::setup() {
   this->setup_called = true;
}

void WindowMotorClass::update() {
   // Called at WindowMotorClass polling rate
   ESP_LOGI(TAG, "motor=%c update", (this->whichMotor == MOTOR_A) ? 'A' : 'B');
}

WhichMotorEnum WindowMotorClass::getWhichMotor() {
   return this->whichMotor;
}

void WindowMotorClass::linkTargetPosition(WCNumber *tpos) {
   this->targetPosition = tpos;
}

void WindowMotorClass::child_setup() {
}

void WindowMotorClass::child_publish_info() {
}

void WindowMotorClass::motor_setup(uint8_t boardid) {
   this->boardid = boardid;
}

void WindowMotorClass::child_update() {
   // Called from parent hub.
   // Could do some synch work here.
   ESP_LOGI(TAG, "motor=%c child_update setup_called=%d", (this->whichMotor == MOTOR_A) ? 'A' : 'B', this->setup_called);
}

} // namespace window_controller
} // namespace esphome
