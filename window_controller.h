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

class WindowMotor {

  public:
    // Constructor
    WindowMotor();
    
    bool setup(uint8_t boardId, bool isMotorA);

    void update();

    // Standard component functions to override
    // void setup() override;
    // void dump_config() override;
    // void update() override;

    void setAllMotorStatus(uint16_t newsts);
 
    // getters
    uint32_t getFaults();
    bool getIsMotorA();

    uint8_t boardId;
    float targetPosition;

  protected:
    // Internal fields definition
    uint32_t faults;
    uint8_t windowNumber;
    uint16_t status;          // statusMask bit is set if this window has work to do
    uint16_t statusMask;
    uint16_t allMotorStatus;  // input from Hass
    bool isMotorA;
    float numRotationsToFullOpen;
    float currentRotationIndex;
    float maxTorqueSeen;
    float current;
    float rpm;
};

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

    void setAllMotorStatus(uint16_t newsts);
    // getters
    uint8_t getBoardId() const;
    uint32_t getFaults();

    WindowMotor motA;
    WindowMotor motB;
    WCMotorUI motuiA;
    WCMotorUI motuiB;
  
    void register_child(WindowControllerClient *obj);

    // This is called by the Python code to link the UI slider to this class
    void set_target_position(WCNumber *n) {
      this->target_position_ = n;
      // Tell the child who its parent is
      this->target_position_->set_parent(this);

      if (whichMotorIsValid(this->target_position_->whichMotor)) {
        if (this->target_position_->whichMotor == MOTOR_A) {
          this->motuiA.target_position = this->target_position_;
        }
        if (this->target_position_->whichMotor == MOTOR_B) {
          this->motuiB.target_position = this->target_position_;
        }
      }

      // Instead of a parent pointer, you can register a callback that
      //   runs the parent's method
      // this->percentage_number_->add_on_state_callback([this](float value) {
      //     this->on_slider_changed(value);
      // });
    }
    void set_max_torque(WCNumber *n) {
      this->max_torque_ = n;
      // Tell the child who its parent is
      this->max_torque_->set_parent(this);

      if (whichMotorIsValid(this->max_torque_->whichMotor)) {
        if (this->max_torque_->whichMotor == MOTOR_A) {
          this->motuiA.max_torque = this->max_torque_;
        }
        if (this->max_torque_->whichMotor == MOTOR_B) {
          this->motuiB.max_torque = this->max_torque_;
        }
      }

      // Instead of a parent pointer, you can register a callback that
      //   runs the parent's method
      // this->percentage_number_->add_on_state_callback([this](float value) {
      //     this->on_slider_changed(value);
      // });
    }

    void set_window_number(WCSensor *s) {
      this->window_number_ = s;
      // Tell the child who its parent is
      this->window_number_->set_parent(this);

      if (whichMotorIsValid(this->window_number_->whichMotor)) {
        if (this->window_number_->whichMotor == MOTOR_A) {
          this->motuiA.window_number = this->window_number_;
        }
        if (this->window_number_->whichMotor == MOTOR_B) {
          this->motuiB.window_number = this->window_number_;
        }
      }

      // Instead of a parent pointer, you can register a callback that
      //   runs the parent's method
      // this->percentage_number_->add_on_state_callback([this](float value) {
      //     this->on_slider_changed(value);
      // });
    }

    void set_faults(WCSensor *s) {
      this->faults_ = s;
      // Tell the child who its parent is
      this->faults_->set_parent(this);

      if (whichMotorIsValid(this->faults_->whichMotor)) {
        if (this->faults_->whichMotor == MOTOR_A) {
          this->motuiA.faults = this->faults_;
        }
        if (this->faults_->whichMotor == MOTOR_B) {
          this->motuiB.faults = this->faults_;
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

    // Controls
    WCNumber *target_position_{ nullptr };
    WCNumber *max_torque_{ nullptr };

    // Status
    WCSensor *window_number_{nullptr};
    WCSensor *faults_{nullptr};
};

}  // namespace esphome::window_controller