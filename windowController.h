#pragma once

#include "esphome/core/component.h"
#include "esphome/core/gpio.h"
//#include "windowMotor.h"
#include "esphome/components/i2c/i2c.h"

// window motor faults
#define WINMOTFAULT_PIN_NULL 0x0

// window controller faults
#define WINCTRLFAULT_BOARDID_PIN_NULL 0x0

#define MAX_BOARD_ID 4

// Namespace definition
namespace esphome::window_controller {

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

    // i2c INA219 current sensor
    i2c::I2CDevice ina219;
    uint8_t boardId;

  protected:
    // Internal fields definition
    InternalGPIOPin *encA_pin_{nullptr};
    InternalGPIOPin *encB_pin_{nullptr};
    InternalGPIOPin *pwm_pin_{nullptr};
    InternalGPIOPin *in1_pin_{nullptr};
    InternalGPIOPin *in2_pin_{nullptr};

    uint32_t faults;
    uint8_t windowNumber;
    uint16_t statusMask;
    uint16_t allMotorStatus;
    bool isMotorA;
    uint8_t targetPositionPercent;
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

class WindowController : public PollingComponent, public i2c::I2CDevice {
  public:
    // Constructor
    WindowController();
    
    // Standard component functions to override
    void setup() override;
    void dump_config() override;
    void update() override;
    void on_safe_shutdown() override;
    void on_shutdown() override;

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
    uint8_t getWindowNumber();
    
    WindowMotor motA;
    WindowMotor motB;
  
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
    bool shutdownImminent{false};
    
};

}  // namespace esphome::window_controller