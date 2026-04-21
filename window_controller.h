#pragma once

#include "esphome/core/component.h"
#include "esphome/core/gpio.h"
#include "esphome/components/i2c/i2c.h"
#include "window_controller_child.h"
#include "esphome/components/number/number.h"

// window motor faults
#define WINMOTFAULT_PIN_NULL 0x0

// window controller faults
#define WINCTRLFAULT_BOARDID_PIN_NULL 0x0

#define MAX_BOARD_ID 4

// Namespace definition
namespace esphome::window_controller {

enum WhichMotorEnum { MOTOR_NONE = 0, MOTOR_A = 1, MOTOR_B = 2 };

// Forward declarations
class WindowControllerClient;
class WindowControllerHub;

// Create a non-abstract number class
class WindowPositionNumber : public number::Number {
public:
  // Store a pointer to the parent hub
  void set_parent(WindowControllerHub *parent) { this->parent_ = parent; }
protected:
  WindowControllerHub *parent_;
  void control(float value) override;
};

class WindowMotor {

  public:
    // Constructor
    WindowMotor();
    
    bool setup(uint8_t boardId, bool isMotorA, InternalGPIOPin *enca_pin, 
              InternalGPIOPin *encb_pin, InternalGPIOPin *pwm_pin, 
              InternalGPIOPin *in1_pin, InternalGPIOPin *in2_pin);

    void update();

    // Standard component functions to override
    // void setup() override;
    // void dump_config() override;
    // void update() override;

    bool assignMotorPins(InternalGPIOPin *enca_pin, 
                        InternalGPIOPin *encb_pin, InternalGPIOPin *pwm_pin, 
                        InternalGPIOPin *in1_pin, InternalGPIOPin *in2_pin);
    bool calcINA219config();
    bool powerdownINA219();
    void calcWinNumAndStsMsk();

    // Add any setters of configuration variables
    // void set_encA_pin(InternalGPIOPin *pin) {encA_pin_ = pin;}
    // void set_encB_pin(InternalGPIOPin *pin) {encB_pin_ = pin;}
    // void set_pwm_pin(InternalGPIOPin *pin) {pwm_pin_ = pin;}
    // void set_in1_pin(InternalGPIOPin *pin) {in1_pin_ = pin;}
    // void set_in2_pin(InternalGPIOPin *pin) {in2_pin_ = pin;}
    // void set_scl_pin(InternalGPIOPin *pin) {scl_pin_ = pin;}
    // void set_sda_pin(InternalGPIOPin *pin) {sda_pin_ = pin;}
    void setAllMotorStatus(uint16_t newsts);
 
    // getters
    bool getBusVoltage(float *bus_voltage_v);
    bool getCurrent(float *current_a);
    bool getShuntVoltage(float *shunt_voltage_mv);
    uint8_t getWindowNumber();
    uint32_t getFaults();
    bool getIsMotorA();

    i2c::I2CDevice ina219;  // current sensor
    uint8_t boardId;
    float targetPosition;

  protected:
    // Internal fields definition
    InternalGPIOPin *encA_pin_{nullptr};
    InternalGPIOPin *encB_pin_{nullptr};
    InternalGPIOPin *pwm_pin_{nullptr};
    InternalGPIOPin *in1_pin_{nullptr};
    InternalGPIOPin *in2_pin_{nullptr};

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
    // ina219 vars
    float shunt_resistance_ohm_;
    float max_current_a_;
    float max_voltage_v_;
    uint32_t calibration_lsb_;
};

class WindowControllerHub : public PollingComponent, public i2c::I2CDevice {
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

    void set_mota_enca_pin(InternalGPIOPin *pin) {mota_enca_pin_ = pin;}
    void set_mota_encb_pin(InternalGPIOPin *pin) {mota_encb_pin_ = pin;}
    void set_mota_pwm_pin(InternalGPIOPin *pin) {mota_pwm_pin_ = pin;}
    void set_mota_in1_pin(InternalGPIOPin *pin) {mota_in1_pin_ = pin;}
    void set_mota_in2_pin(InternalGPIOPin *pin) {mota_in2_pin_ = pin;}

    void set_motb_enca_pin(InternalGPIOPin *pin) {motb_enca_pin_ = pin;}
    void set_motb_encb_pin(InternalGPIOPin *pin) {motb_encb_pin_ = pin;}
    void set_motb_pwm_pin(InternalGPIOPin *pin) {motb_pwm_pin_ = pin;}
    void set_motb_in1_pin(InternalGPIOPin *pin) {motb_in1_pin_ = pin;}
    void set_motb_in2_pin(InternalGPIOPin *pin) {motb_in2_pin_ = pin;}
    void setAllMotorStatus(uint16_t newsts);
    // getters
    uint8_t getBoardId() const;
    uint32_t getFaults();

    WindowMotor motA;
    WindowMotor motB;
  
    void register_child(WindowControllerClient *obj);

    // This is called by the Python code to link the UI slider to this class
    void set_speed_slider(WindowPositionNumber *n) {
      this->speed_slider_ = n;
      // Tell the child who its parent is
      this->speed_slider_->set_parent(this);

      // Instead of a parent pointer, you can register a callback that
      //   runs the parent's method
      // this->percentage_number_->add_on_state_callback([this](float value) {
      //     this->on_slider_changed(value);
      // });
    }
    void set_calibration_slider(WindowPositionNumber *n) {
      this->calibration_slider_ = n;
      // Tell the child who its parent is
      this->calibration_slider_->set_parent(this);

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
    // Internal fields definition
    InternalGPIOPin *boardid0_pin_{nullptr};
    InternalGPIOPin *boardid1_pin_{nullptr};
    InternalGPIOPin *boardid2_pin_{nullptr};

    InternalGPIOPin *mota_enca_pin_{nullptr};
    InternalGPIOPin *mota_encb_pin_{nullptr};
    InternalGPIOPin *mota_pwm_pin_{nullptr};
    InternalGPIOPin *mota_in1_pin_{nullptr};
    InternalGPIOPin *mota_in2_pin_{nullptr};

    InternalGPIOPin *motb_enca_pin_{nullptr};
    InternalGPIOPin *motb_encb_pin_{nullptr};
    InternalGPIOPin *motb_pwm_pin_{nullptr};
    InternalGPIOPin *motb_in1_pin_{nullptr};
    InternalGPIOPin *motb_in2_pin_{nullptr};

    uint8_t boardId{0};
    uint32_t faults{0};
    bool shutdownImminent{false};

    WindowPositionNumber *speed_slider_{ nullptr };
    WindowPositionNumber *calibration_slider_{ nullptr };

};

}  // namespace esphome::window_controller