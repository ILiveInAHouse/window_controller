#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"

#include "../window_controller.h"
#include "../window_controller_child.h"
#include "../wc_whichmotor.h"
#include "../wc_number.h"
#include "../wc_motorui.h"

namespace esphome {
namespace window_controller {

class WindowMotorClass : public WindowControllerClient, public PollingComponent {
   public:
      void setup() override;
      void update() override;
      void calcWinNumAndStsMsk();
      void set_whichMotor(WhichMotorEnum whichMotor_) {
         this->whichMotor = whichMotor_;
      }
      WhichMotorEnum whichMotor;
      void child_setup(WCMotorUI *ui);
      void child_publish_info();
      void child_update();
      void child_dump_config();
      WhichMotorEnum getWhichMotor();
      WCMotorUI *ui;
      uint16_t statusMask;
      void set_enca_pin(InternalGPIOPin *pin) {enca_pin_ = pin;}
      void set_encb_pin(InternalGPIOPin *pin) {encb_pin_ = pin;}
      void set_pwm_pin(InternalGPIOPin *pin) {pwm_pin_ = pin;}
      void set_in1_pin(InternalGPIOPin *pin) {in1_pin_ = pin;}
      void set_in2_pin(InternalGPIOPin *pin) {in2_pin_ = pin;}

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

};

} // namespace window_controller
} // namespace esphome