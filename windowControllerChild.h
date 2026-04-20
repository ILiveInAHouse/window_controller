#pragma once

#include "esphome/core/helpers.h"

namespace esphome {
namespace window_controller {

// Forward declare WindowController
class WindowController;

class WindowControllerClient : public Parented<WindowController> {
 public:
//   virtual void on_status(const BedjetStatusPacket *data) = 0;
//   virtual void on_bedjet_state(bool is_ready) = 0;
  virtual void child_setup() = 0;
  virtual void publish_info() = 0;

 protected:
  friend WindowController;
//   virtual std::string describe() = 0;
};

}  // namespace window_controller
}  // namespace esphome
