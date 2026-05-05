#pragma once

#include "esphome/core/component.h"
#include "esphome/core/gpio.h"
#include "window_controller_child.h"
#include "esphome/components/number/number.h"
#include "esphome/components/sensor/sensor.h"
#include "wc_number.h"
#include "wc_whichmotor.h"
#include "wc_sensor.h"
#include "wc_motorui.h"

// window motor faults
#define WINMOTFAULT_PIN_NULL 0x0

// window controller faults
#define WINCTRLFAULT_BOARDID_PIN_NULL 0x0

#define MAX_BOARD_ID 4
#define FUNC_OK 1
#define FUNC_FAIL 0

// Namespace definition
namespace esphome::window_controller {

// Forward declarations
class WindowControllerClient;
class WindowControllerHub;

class WindowControllerHub : public PollingComponent {
  public:
    // Constructor
    WindowControllerHub();
    
    // Standard component functions to override
    void setup() override;
    void dump_config() override;
    void update() override;
    void on_safe_shutdown() override;
    void on_shutdown() override;

    void print_number_change(float new_n);

    // Add any setters of configuration variables
    void set_boardid0_pin(InternalGPIOPin *pin) {boardid0_pin_ = pin;}
    void set_boardid1_pin(InternalGPIOPin *pin) {boardid1_pin_ = pin;}
    void set_boardid2_pin(InternalGPIOPin *pin) {boardid2_pin_ = pin;}

    WCMotorUI motuiA;
    WCMotorUI motuiB;
  
    void register_child(WindowControllerClient *obj);

    // This is called by the Python code to link the UI slider to this class
    void set_target_position(WCNumber *n) {
      WCNumber *n_ = n;
      // Tell the child who its parent is
      n_->set_parent(this);

      if (whichMotorIsValid(n_->whichMotor)) {
        if (n_->whichMotor == MOTOR_A) {
          this->motuiA.target_position_Number = n_;
        }
        if (n_->whichMotor == MOTOR_B) {
          this->motuiB.target_position_Number = n_;
        }
      }

      // Instead of a parent pointer, you can register a callback that
      //   runs the parent's method
      // this->percentage_number_->add_on_state_callback([this](float value) {
      //     this->on_slider_changed(value);
      // });
    }
    void set_max_torque(WCNumber *n) {
      WCNumber *n_ = n;
      // Tell the child who its parent is
      n_->set_parent(this);

      if (whichMotorIsValid(n_->whichMotor)) {
        if (n_->whichMotor == MOTOR_A) {
          this->motuiA.max_torque_Number = n_;
        }
        if (n_->whichMotor == MOTOR_B) {
          this->motuiB.max_torque_Number = n_;
        }
      }

      // Instead of a parent pointer, you can register a callback that
      //   runs the parent's method
      // this->percentage_number_->add_on_state_callback([this](float value) {
      //     this->on_slider_changed(value);
      // });
    }

    void set_window_number(WCSensor *s) {
      // this->window_number_Sensor = s;
      WCSensor *s_ = s;
      // Tell the child who its parent is
      s_->set_parent(this);

      if (whichMotorIsValid(s_->whichMotor)) {
        if (s_->whichMotor == MOTOR_A) {
          this->sensa++;
          this->motuiA.window_number_Sensor = s_;
        }
        if (s_->whichMotor == MOTOR_B) {
          this->sensb++;
          this->motuiB.window_number_Sensor = s_;
        }
      }

      // Instead of a parent pointer, you can register a callback that
      //   runs the parent's method
      // this->percentage_number_->add_on_state_callback([this](float value) {
      //     this->on_slider_changed(value);
      // });
    }

    void set_faults(WCSensor *s) {
      WCSensor *s_ = s;
      // Tell the child who its parent is
      s_->set_parent(this);

      if (whichMotorIsValid(s_->whichMotor)) {
        if (s_->whichMotor == MOTOR_A) {
          this->motuiA.faults_Sensor = s_;
        }
        if (s_->whichMotor == MOTOR_B) {
          this->motuiB.faults_Sensor = s_;
        }
      }

      // Instead of a parent pointer, you can register a callback that
      //   runs the parent's method
      // this->percentage_number_->add_on_state_callback([this](float value) {
      //     this->on_slider_changed(value);
      // });
    }

  protected:
    std::vector<WindowControllerClient *> children_;
    void all_children_publish_info();
    void all_children_update();
    void all_children_dump_config();
    // Internal fields definition
    InternalGPIOPin *boardid0_pin_{nullptr};
    InternalGPIOPin *boardid1_pin_{nullptr};
    InternalGPIOPin *boardid2_pin_{nullptr};

    uint8_t boardId{0};
    uint32_t faults{0};
    bool shutdownImminent{false};
    int sensa{0};
    int sensb{0};

};

}  // namespace esphome::window_controller