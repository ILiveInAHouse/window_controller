#include "windowController_sensor.h"

#include "esphome/core/log.h"

namespace esphome {
namespace window_controller {

static const char *const TAG = "windowcontroller.sensor";

uint8_t WindowControllerSensor::getWindowNumber() {
  return this->parent_->getWindowNumber();
}

void WindowControllerSensor::update() {
   uint8_t winNum = this->getWindowNumber();
   ESP_LOGD(TAG, "'%s': Got Window Number=%d", this->get_name().c_str(), winNum);
   this->publish_state(winNum);
}

void WindowControllerSensor::dump_config() {
  LOG_SENSOR("  ", "Window Controller Sensor", this);
//   ESP_LOGCONFIG(TAG,
//                 "    Multiplexer: %u\n"
//                 "    Gain: %u\n"
//                 "    Resolution: %u\n"
//                 "    Sample rate: %u",
//                 this->multiplexer_, this->gain_, this->resolution_, this->samplerate_);
}

}  // namespace window_controller
}  // namespace esphome
