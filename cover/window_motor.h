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

#define INVALID_ENCODER_LAST_CALLBACK_US (uint32_t)(0xfffffffff)

#define MOTFAULT_INA219_INIT 0x1
#define MOTFAULT_PIN_INIT 0x2
#define MOTFAULT_INA219_READ 0x4

// poll rate is set in __init__.py
#define POLL_RATE_MS 50u

enum MotorDriverModeEnum { MOTMODE_CW=0, MOTMODE_CCW=1, MOTMODE_SHORTBRAKE=2, MOTMODE_STOP=3 };
enum WindowDirectionEnum { WINDIR_OPEN=0, WINDIR_CLOSE=1, WINDIR_STOP=2 };
enum WindowStateEnum { 
   WINST_UNINITIALIZED=0, 
   WINST_INIT_TRY_OPEN=1, 
   WINST_INIT_TRY_CLOSE=2, 
   WINST_OPEN=3, 
   WINST_CLOSED=4, 
   WINST_BETWEEN_STOPPED=5, 
   WINST_BETWEEN_MOVING=6,
   WINST_OPENING=7, 
   WINST_CLOSING=8 };

struct winParams_t {
   float openMaxCurrent;
   float closeMaxCurrent;
   float startMaxCurrent;
};

class WindowMotorClass : public WindowControllerClient, public PollingComponent, public i2c::I2CDevice {
   public:
      // Constructor
      WindowMotorClass();
      void setup() override;
      void update() override;
      void on_shutdown() override;
      void on_safe_shutdown() override;
      void dump_config() override;
      float get_setup_priority() const override { 
         return esphome::setup_priority::DATA; // Lower priority than HARDWARE
      }
      // i2c
      i2c::I2CDevice ina219;  // current sensor
      bool calcINA219config();
      bool powerdownINA219();
      bool getBusVoltage(float *bus_voltage_v);
      bool getCurrent(float *current_a);
      bool getShuntVoltage(float *shunt_voltage_mv);

      // Control Callbacks
      void controlTargetPosition(float value);
      void controlAllMotorStatus(float value);
      // Encoder Callback
      void encoderListener(int32_t val);

      float estimatedCurrentPosition();

      // Pin Control
      bool setup_pins();
      void setMotorDriverMode(MotorDriverModeEnum mode);
      void pollMotorMove();
      void setFault(uint32_t fault_bit);
      void setMotorStatus(uint16_t sts);
      void setEstPosition(float pos);
      void runPwm();
      void runCurrentManagement(float speed);
      void stopMotor();
      void calcWinNumAndStsMsk();
      void setWhichMotor(WhichMotorEnum whichMotor_) {this->whichMotor = whichMotor_;}
      WhichMotorEnum getWhichMotor() {return this->whichMotor;}
      void child_setup(WCMotorUI *ui);
      void child_publish_info();
      void child_sync_update();
      WCMotorUI *ui;
      void set_in1_pin(InternalGPIOPin *pin) {in1_pin_ = pin;}
      void set_in2_pin(InternalGPIOPin *pin) {in2_pin_ = pin;}
      bool my_turn_to_move();
      
#define ENCODER_COUNTER_INIT INT_MAX
   protected:
      uint32_t faults={0};
      WCNumber *targetPosition;
      uint8_t windowNumber;
      WhichMotorEnum whichMotor;
      uint16_t statusMask;
      InternalGPIOPin *in1_pin_={nullptr};
      InternalGPIOPin *in2_pin_={nullptr};
      MotorDriverModeEnum motmode={MOTMODE_STOP};
      void setWindowDirection(WindowDirectionEnum dir);
      WindowDirectionEnum windir={WINDIR_STOP};
      // Encoder 
      int32_t encoderLastCounter={0};
      int32_t encoderCounterAtClosed={ENCODER_COUNTER_INIT};
      int32_t encoderCounterAtOpen={ENCODER_COUNTER_INIT};
      uint32_t encoderLastCallback_us={INVALID_ENCODER_LAST_CALLBACK_US};
      float encoderSpeed_stepspers={0.0f}; // steps per second

      // ina219 config vars
      float shunt_resistance_ohm_;
      float max_current_a_;
      float max_voltage_v_;
      uint32_t calibration_lsb_;
      // ina219 monitor vars
      float largest_current_ever_a={0.0f};

      bool shutdownImminent={false};
      float duty={0.0f};
      WindowStateEnum winstate={WINST_UNINITIALIZED};
      bool initTryOpenSuccess={false};
      bool initTryCloseSuccess={false};
      uint32_t movementStartupCounter={0};
};

} // namespace window_controller
} // namespace esphome