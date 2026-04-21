#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"

#include "esphome/components/number/number.h"

#include "../window_controller.h"
#include "../window_controller_child.h"

namespace esphome {
namespace window_controller {

/// Internal holder class that is in instance of Sensor so that the hub can create individual sensors.
class WindowControllerNumber : public WindowControllerClient, public number::Number, public PollingComponent {
 public:
  WindowControllerNumber();
  void dump_config() override;
  void child_setup();
  void update() override;
  void control(float val) override;
  bool floatsNotEqual(float a, float b, float delta);
  void set_whichMotor(WhichMotorEnum whichMotor_) {
    this->whichMotor = whichMotor_;
  }
  void set_target_position_number(number::Number *target_position_number) {
    this->target_position_number_ = target_position_number;
  }
  void publish_info();
  WhichMotorEnum whichMotor;
  WindowMotor *motorClassPtr;

 protected:
    number::Number *target_position_number_{nullptr};
    int initVal;

};

}  // namespace window_controller
}  // namespace esphome

/*
callstack
BedJetHub::update() -> this->dispatch_status_()
BedJetHub::dispatch_status_() -> for each child, child->on_status(status)
BedJetSensor::on_status(BedjetStatusPacket*) -> this->outlet_temp_sensor_->publish_state(temp)

WindowController::update() -> this->publish_info()
WindowController::publish_info() -> for each child, child->
*/