#pragma once

#include "esphome/core/helpers.h"

namespace esphome {
namespace window_controller {

// Forward declare WindowControllerHub
class WindowControllerHub;

class WindowControllerClient : public Parented<WindowControllerHub> {
 public:
//   virtual void on_status(const BedjetStatusPacket *data) = 0;
//   virtual void on_bedjet_state(bool is_ready) = 0;
  virtual void child_setup() = 0;
  virtual void child_publish_info() = 0;
  virtual void child_update() = 0;

 protected:
  friend WindowControllerHub;
//   virtual std::string describe() = 0;
};

}  // namespace window_controller
}  // namespace esphome
