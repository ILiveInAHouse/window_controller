#include "windowController_sensor.h"

#include "esphome/core/log.h"

namespace esphome {
namespace window_controller {

static const char *const TAG = "windowcontroller.sensor";

uint8_t WindowControllerSensor::getWindowNumber() {
  return this->parent_->getWindowNumber();
}

void WindowControllerSensor::publish_info() {
   uint8_t winNum_ = this->getWindowNumber();
   ESP_LOGD(TAG, "'Got Window Number=%d", winNum_);
   this->window_number_sensor_->publish_state(winNum_);
   uint32_t faults_  = this->parent_->getFaults();
   this->faults_sensor_->publish_state(faults_);
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
