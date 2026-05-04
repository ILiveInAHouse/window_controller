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
      WhichMotorEnum getWhichMotor();
      WCMotorUI *ui;
      uint16_t statusMask;

   protected:
      bool setup_called = false;
      uint32_t faults;
      uint8_t boardid;
      WCNumber *targetPosition;
      uint8_t windowNumber;

};

} // namespace window_controller
} // namespace esphome