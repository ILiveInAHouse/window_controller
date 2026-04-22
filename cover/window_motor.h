#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"

#include "../window_controller.h"
#include "../window_controller_child.h"
#include "../wc_whichmotor.h"
#include "../wc_number.h"

namespace esphome {
namespace window_controller {

class WindowMotorClass : public WindowControllerClient, public PollingComponent {
   public:
      void setup() override;
      void update() override;
      void set_whichMotor(WhichMotorEnum whichMotor_) {
         this->whichMotor = whichMotor_;
      }
      WhichMotorEnum whichMotor;
      void motor_setup(uint8_t boardid);
      void child_setup();
      void child_publish_info();
      void child_update();
      WhichMotorEnum getWhichMotor();
      void linkTargetPosition(WCNumber *tpos);

   protected:
      bool setup_called = false;
      uint32_t faults;
      uint8_t boardid;
      WCNumber *targetPosition;

};

} // namespace window_controller
} // namespace esphome