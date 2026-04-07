#pragma once

#include "esphome/core/component.h"
#include "esphome/core/gpio.h"
//#include "windowMotor.h"
#include "esphome/components/i2c/i2c.h"

// window motor controller faults
#define WINCTRLFAULT_BOARDID_PIN_NULL 0x0

// Namespace definition
namespace esphome::window_controller {

class WindowMotor {

  public:
    // Constructor
    WindowMotor();
    
    // Standard component functions to override
    // void setup() override;
    // void dump_config() override;
    // void update() override;

    bool assignMotorPins(InternalGPIOPin *enca_pin, 
                        InternalGPIOPin *encb_pin, InternalGPIOPin *pwm_pin, 
                        InternalGPIOPin *in1_pin, InternalGPIOPin *in2_pin);

    // Add any setters of configuration variables
    // void set_encA_pin(InternalGPIOPin *pin) {encA_pin_ = pin;}
    // void set_encB_pin(InternalGPIOPin *pin) {encB_pin_ = pin;}
    // void set_pwm_pin(InternalGPIOPin *pin) {pwm_pin_ = pin;}
    // void set_in1_pin(InternalGPIOPin *pin) {in1_pin_ = pin;}
    // void set_in2_pin(InternalGPIOPin *pin) {in2_pin_ = pin;}
    // void set_scl_pin(InternalGPIOPin *pin) {scl_pin_ = pin;}
    // void set_sda_pin(InternalGPIOPin *pin) {sda_pin_ = pin;}
 
    // getters
  protected:
    // Internal fields definition
    InternalGPIOPin *encA_pin_{nullptr};
    InternalGPIOPin *encB_pin_{nullptr};
    InternalGPIOPin *pwm_pin_{nullptr};
    InternalGPIOPin *in1_pin_{nullptr};
    InternalGPIOPin *in2_pin_{nullptr};

    uint32_t faults;
    uint8_t targetPositionPercent;
    float numRotationsToFullOpen;
    float currentRotationIndex;
    float maxTorqueSeen;
    float current;
    float rpm;
};

class WindowController : public PollingComponent, public i2c::I2CDevice {
  public:
    // Constructor
    WindowController();
    
    // Standard component functions to override
    void setup() override;
    void dump_config() override;
    void update() override;
    void on_powerdown() override;

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
    // getters
    uint8_t getBoardId() const;
  
  protected:
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
    float shunt_resistance_ohm_;
    float max_current_a_;
    float max_voltage_v_;
    uint32_t calibration_lsb_;
    WindowMotor motA;
    WindowMotor motB;

    bool calcINA219config(uint16_t *config);
};

}  // namespace esphome::window_controller