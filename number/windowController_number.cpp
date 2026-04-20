#include "windowController_number.h"

#include "esphome/core/log.h"

namespace esphome {
namespace window_controller {

static const char *const TAG = "windowcontroller.number";

WindowControllerNumber::WindowControllerNumber() {
  this->initVal = 0;
  this->whichMotor = MOTOR_NONE;
}

void WindowControllerNumber::child_setup() {
  // setup() doesn't seem to run.  Don't know why.
  // FIXME: is it because windowControllerChild.h doesn't define setup()?
  this->initVal = 2;
  if (this->whichMotor == MOTOR_A) {
    this->motorClassPtr = &this->parent_->motA;
  } else {
    this->motorClassPtr = &this->parent_->motB;
  }
}

void WindowControllerNumber::control(float val) {
  ESP_LOGD(TAG, "initVal=%d whichMotor=%d motorClassPtr=%p motA=%p motB=%p", 
    this->initVal, this->whichMotor, this->motorClassPtr, this->parent_->motA, this->parent_->motB);
  // if (this->motorClassPtr) {
  //   ESP_LOGD(TAG, "control motor%c: isMotorA: %d targetPos=%f", 
  //       (this->whichMotor == MOTOR_A) ? 'A' : 'B', this->motorClassPtr->getIsMotorA(), 
  //       val);
  // } else {
  //   ESP_LOGD(TAG, "control %f: motorClassPtr is null", val);
  // }
  this->publish_state(val);
}

bool WindowControllerNumber::floatsNotEqual(float a, float b, float delta) {
  if (std::isnan(a) || std::isnan(b) || std::isnan(delta)) {
    return true;
  }
  if (std::abs(a-b) > delta) {
    return true;
  }
  return false;
}

void WindowControllerNumber::publish_info() {
  if (this->motorClassPtr) {
    //float targetPos_ = this->motorClassPtr->targetPosition;
    // ESP_LOGD(TAG, "winNum=%d get_state=%f state-winNum=%f", winNum_, this->window_number_sensor_->get_state(), winNum_ - this->window_number_sensor_->get_state());
    // if (this->floatsNotEqual(this->target_position_->state, targetPos_, 0.1f)) {
      //ESP_LOGD(TAG, "motor%c: targetPos=%f", (this->whichMotor == MOTOR_A) ? 'A' : 'B', targetPos_);
      //this->target_position_number_->publish_state(targetPos_);
    // }
  }
}

void WindowControllerNumber::update() {

}

void WindowControllerNumber::dump_config() {
//  LOG_SENSOR("  ", "Window Controller Number", this);
//   ESP_LOGCONFIG(TAG,
//                 "    Multiplexer: %u\n"
//                 "    Gain: %u\n"
//                 "    Resolution: %u\n"
//                 "    Sample rate: %u",
//                 this->multiplexer_, this->gain_, this->resolution_, this->samplerate_);
}

}  // namespace window_controller
}  // namespace esphome
