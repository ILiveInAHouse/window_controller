#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"

#include "../window_controller.h"
#include "../window_controller_child.h"

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
      void child_setup();
      void child_publish_info();
      void child_update();
   protected:
      bool setup_called = false;
};

} // namespace window_controller
} // namespace esphome