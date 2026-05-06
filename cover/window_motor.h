#pragma once

#include "esphome.h"
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

#define MOTFAULT_INA219_INIT 0x1
#define MOTFAULT_PIN_INIT 0x2

enum MotorDriverModeEnum { MOTMODE_CW = 0, MOTMODE_CCW = 1, MOTMODE_SHORTBRAKE = 2, MOTMODE_STOP=3 };

class WindowMotorClass : public WindowControllerClient, public PollingComponent, public i2c::I2CDevice {
   public:
      // Constructor
      WindowMotorClass();
      void setup() override;
      void update() override;
      void on_shutdown() override;
      void on_safe_shutdown() override;
      void dump_config() override;
      // i2c
      i2c::I2CDevice ina219;  // current sensor
      bool calcINA219config();
      bool powerdownINA219();
      bool getBusVoltage(float *bus_voltage_v);
      bool getCurrent(float *current_a);
      bool getShuntVoltage(float *shunt_voltage_mv);

      // Controls
      void controlTargetPosition(float value);
      void controlAllMotorStatus(float value);

      // Pin Control
      bool setup_pins();
      void setMotorDriverMode(MotorDriverModeEnum mode);
      void pollMotorMove();
      void setFault(uint32_t fault_bit);
      void setMotorStatus(uint16_t sts);
      void setEstPosition(float pos);
      void setPwm(float duty);
      void calcWinNumAndStsMsk();
      void setWhichMotor(WhichMotorEnum whichMotor_) {this->whichMotor = whichMotor_;}
      WhichMotorEnum getWhichMotor() {return this->whichMotor;}
      void child_setup(WCMotorUI *ui);
      void child_publish_info();
      void child_sync_update();
      WCMotorUI *ui;
      void set_enca_pin(InternalGPIOPin *pin) {enca_pin_ = pin;}
      void set_encb_pin(InternalGPIOPin *pin) {encb_pin_ = pin;}
      void set_pwm_pin(output::FloatOutput *pin) {pwm_pin_ = pin;}
      void set_in1_pin(InternalGPIOPin *pin) {in1_pin_ = pin;}
      void set_in2_pin(InternalGPIOPin *pin) {in2_pin_ = pin;}
      WindowMotorClass(output::FloatOutput *pwm_pin) : pwm_pin_(pwm_pin) {}


   protected:
      uint32_t faults{0};
      WCNumber *targetPosition;
      uint8_t windowNumber;
      WhichMotorEnum whichMotor;
      uint16_t statusMask;
      InternalGPIOPin *enca_pin_{nullptr};
      InternalGPIOPin *encb_pin_{nullptr};
      output::FloatOutput *pwm_pin_{nullptr};
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