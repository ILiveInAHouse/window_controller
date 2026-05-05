#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/components/i2c/i2c.h"
#include "../window_controller.h"
#include "../window_controller_child.h"
#include "../wc_whichmotor.h"
#include "../wc_number.h"
#include "../wc_motorui.h"

namespace esphome {
namespace window_controller {

class WindowMotorClass : public WindowControllerClient, public PollingComponent, public i2c::I2CDevice {
   public:
      // Constructor
      WindowMotorClass();
      void setup() override;
      void update() override;
      void on_shutdown() override;
      void calcWinNumAndStsMsk();
      void set_whichMotor(WhichMotorEnum whichMotor_) {
         this->whichMotor = whichMotor_;
      }
      WhichMotorEnum whichMotor;
      void child_setup(WCMotorUI *ui);
      void child_publish_info();
      void child_update();
      void child_dump_config();
      void child_on_safe_shutdown();
      WhichMotorEnum getWhichMotor();
      WCMotorUI *ui;
      uint16_t statusMask;
      void set_enca_pin(InternalGPIOPin *pin) {enca_pin_ = pin;}
      void set_encb_pin(InternalGPIOPin *pin) {encb_pin_ = pin;}
      void set_pwm_pin(InternalGPIOPin *pin) {pwm_pin_ = pin;}
      void set_in1_pin(InternalGPIOPin *pin) {in1_pin_ = pin;}
      void set_in2_pin(InternalGPIOPin *pin) {in2_pin_ = pin;}
      i2c::I2CDevice ina219;  // current sensor
    bool calcINA219config();
    bool powerdownINA219();
    bool getBusVoltage(float *bus_voltage_v);
    bool getCurrent(float *current_a);
    bool getShuntVoltage(float *shunt_voltage_mv);


   protected:
      bool setup_called = false;
      uint32_t faults;
      uint8_t boardid;
      WCNumber *targetPosition;
      uint8_t windowNumber;
      InternalGPIOPin *enca_pin_{nullptr};
      InternalGPIOPin *encb_pin_{nullptr};
      InternalGPIOPin *pwm_pin_{nullptr};
      InternalGPIOPin *in1_pin_{nullptr};
      InternalGPIOPin *in2_pin_{nullptr};
      // ina219 vars
      float shunt_resistance_ohm_;
      float max_current_a_;
      float max_voltage_v_;
      uint32_t calibration_lsb_;
    bool shutdownImminent{false};
};

} // namespace window_controller
} // namespace esphome