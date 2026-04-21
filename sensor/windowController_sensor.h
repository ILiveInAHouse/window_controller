#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"

#include "esphome/components/sensor/sensor.h"

#include "../window_controller.h"
#include "../window_controller_child.h"

namespace esphome {
namespace window_controller {

/// Internal holder class that is in instance of Sensor so that the hub can create individual sensors.
class WindowControllerSensor : public WindowControllerClient, public PollingComponent {
 public:
  void dump_config() override;
  void setup() override;
  void update() override;
  bool floatsNotEqual(float a, float b, float delta);
  void set_window_number_sensor(sensor::Sensor *window_number_sensor) {
    this->window_number_sensor_ = window_number_sensor;
  }
  void set_faults_sensor(sensor::Sensor *faults_sensor) {
    this->faults_sensor_ = faults_sensor;
  }
  void set_whichMotor(WhichMotorEnum whichMotor_) {
    this->whichMotor = whichMotor_;
  }
  void child_setup();
  void publish_info();
  WhichMotorEnum whichMotor;
  WindowMotor *motorClassPtr;

 protected:
    sensor::Sensor *window_number_sensor_{nullptr};
    sensor::Sensor *faults_sensor_{nullptr};

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