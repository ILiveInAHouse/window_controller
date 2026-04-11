#include "windowController_sensor.h"

#include "esphome/core/log.h"

namespace esphome {
namespace window_controller {

static const char *const TAG = "windowcontroller.sensor";

uint8_t WindowControllerSensor::getWindowNumber() {
  return this->parent_->getWindowNumber();
}

void WindowControllerSensor::setup() {
  if (this->whichMotor == MOTOR_A) {
    this->motorClassPtr = &this->parent_->motA;
  } else {
    this->motorClassPtr = &this->parent_->motB;
  }
}

bool floatsNotEqual(float a, float b, float delta) {
  if (std::isnan(a) || std::isnan(b) || std::isnan(delta)) {
    return true;
  }
  if (std::abs(a-b) > delta) {
    return true;
  }
  return false;
}

void WindowControllerSensor::publish_info() {
  uint8_t winNum_ = this->motorClassPtr->getWindowNumber();
  // ESP_LOGD(TAG, "winNum=%d get_state=%f state-winNum=%f", winNum_, this->window_number_sensor_->get_state(), winNum_ - this->window_number_sensor_->get_state());
  if (floatsNotEqual(this->window_number_sensor_->get_state(), winNum_, 0.2f)) {
    ESP_LOGD(TAG, "motor%c: winNum=%d", (this->whichMotor == MOTOR_A) ? 'A' : 'B', winNum_);
    this->window_number_sensor_->publish_state(winNum_);
  }
  uint32_t faults_  = this->parent_->getFaults();
  if (floatsNotEqual(this->faults_sensor_->get_state(), faults_, 0.2f)) {
    this->faults_sensor_->publish_state(faults_);
  }
}

void WindowControllerSensor::update() {

}

void WindowControllerSensor::dump_config() {
//  LOG_SENSOR("  ", "Window Controller Sensor", this);
//   ESP_LOGCONFIG(TAG,
//                 "    Multiplexer: %u\n"
//                 "    Gain: %u\n"
//                 "    Resolution: %u\n"
//                 "    Sample rate: %u",
//                 this->multiplexer_, this->gain_, this->resolution_, this->samplerate_);
}

}  // namespace window_controller
}  // namespace esphome
