#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"

#include "../window_controller.h"
#include "../window_controller_child.h"

namespace esphome {
namespace window_controller {

class WindowMotorClass : public WindowControllerClient, public PollingComponent {
   public:
      void child_setup();
      void set_whichMotor(WhichMotorEnum whichMotor_) {
         this->whichMotor = whichMotor_;
      }
      WhichMotorEnum whichMotor;
      void update() override;
      void publish_info() override;
};
}
}