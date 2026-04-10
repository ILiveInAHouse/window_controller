#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"

#include "esphome/components/sensor/sensor.h"

#include "../windowController.h"

namespace esphome {
namespace window_controller {

/// Internal holder class that is in instance of Sensor so that the hub can create individual sensors.
class WindowControllerSensor : public sensor::Sensor,
                      public PollingComponent,
                      public Parented<WindowController> {
 public:
  void update() override;
  uint8_t getWindowNumber();

  void dump_config() override;

 protected:
};

}  // namespace window_controller
}  // namespace esphome
