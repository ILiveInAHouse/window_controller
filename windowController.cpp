#include "esphome/core/log.h"
#include "esphome/core/hal.h"
#include "windowController.h"

namespace esphome::window_controller {

static const char *TAG = "window_controller.component";

static const uint8_t INA219_READ = 0x01;
static const uint8_t INA219_REGISTER_CONFIG = 0x00;
static const uint8_t INA219_REGISTER_SHUNT_VOLTAGE = 0x01;
static const uint8_t INA219_REGISTER_BUS_VOLTAGE = 0x02;
static const uint8_t INA219_REGISTER_POWER = 0x03;
static const uint8_t INA219_REGISTER_CURRENT = 0x04;
static const uint8_t INA219_REGISTER_CALIBRATION = 0x05;

// INA219 config info:
// 0b00000xxxx0000000 << 7 Bus ADC Resolution/Averaging
// 0b000000000xxxx000 << 3 Shunt ADC Resolution/Averaging

// Value  Resolution, Averaging, Conversion
// 0b0X00 -> 9 bit, 1 sample, 84 µs
// 0b0X01 -> 10 bit, 1 sample, 148 µs
// 0b0X10 -> 11 bit, 1 sample, 276 µs
// 0b0X11 -> 12 bit, 1 sample, 532 µs
// 0b1001 -> 12 bit, 2 samples, 1.06 ms
// 0b1010 -> 12 bit, 4 samples, 2.13 ms
// 0b1011 -> 12 bit, 8 samples, 4.26 ms
// 0b1100 -> 12 bit, 16 samples, 8.51 ms
// 0b1101 -> 12 bit, 32 samples, 17.02 ms
// 0b1110 -> 12 bit, 64 samples, 34.05 ms
// 0b1111 -> 12 bit, 128 samples, 68.10 ms <--

// 0b0000000000000xxx << 0 Mode (Bus and Shunt continuous -> 0b111)

WindowMotor::WindowMotor() {
    this->faults = 0;
}

#define ASSIGN_OK 1
#define ASSIGN_FAIL 0
bool WindowMotor::assignMotorPins(InternalGPIOPin *enca_pin, 
                        InternalGPIOPin *encb_pin, InternalGPIOPin *pwm_pin, 
                        InternalGPIOPin *in1_pin, InternalGPIOPin *in2_pin) {
    if ((enca_pin == nullptr) || 
        (encb_pin == nullptr) || 
        (pwm_pin == nullptr) || 
        (in1_pin == nullptr) || 
        (in2_pin == nullptr)) {
        return ASSIGN_FAIL;
    }
    this->encA_pin_ = enca_pin;
    this->encB_pin_ = encb_pin;
    this->pwm_pin_ = pwm_pin;
    this->in1_pin_ = in1_pin;
    this->in2_pin_ = in2_pin;
    return ASSIGN_OK;
}

WindowController::WindowController() {
    // Constructor
    // Initialize class fields and configurations
}

#define CALC_OK 1
#define CALC_FAIL 0
bool WindowController::calcINA219config(uint16_t *config, esphome::i2c::I2CDevice dev) {
    // INA219 config
    // Bus ADC and Shunt ADC 12 bit+128 samples
    *config = 0x0000;
    // Continuous operation of Bus and Shunt ADCs
    *config |= 0b0000000000000111;
    // Bus ADC and Shunt ADC 12 bit+128 samples -> 68.10 ms
    *config |= 0b0000011110000000;
    *config |= 0b0000000001111000;
    // default=0.1, min=0.0, max=32.0
    this->shunt_resistance_ohm_ = 0.1;
    // default=3.2, min=0.0
    this->max_current_a_ = 6.0;
    const float shunt_max_voltage = this->shunt_resistance_ohm_ * this->max_current_a_;

    // default=32.0, min=0.0, max=32.0
    this->max_voltage_v_ = 12.0;
    // 0b00x0000000000000 << 13 Bus Voltage Range (0 -> 16V, 1 -> 32V)
    bool bus_32v_range = this->max_voltage_v_ > 16.0f || shunt_max_voltage > 0.16f;
    float multiplier;
    if (bus_32v_range) {
        *config |= 0b0010000000000000;
        multiplier = 0.5f;
    } else {
        *config |= 0b0000000000000000;
        multiplier = 1.0f;
    }

    // 0b000xx00000000000 << 11 Shunt Voltage Gain (0b00 -> 40mV, 0b01 -> 80mV, 0b10 -> 160mV, 0b11 -> 320mV)
    uint16_t shunt_gain;
    if (shunt_max_voltage * multiplier <= 0.02f) {
        shunt_gain = 0b00;  // 40mV
    } else if (shunt_max_voltage * multiplier <= 0.04f) {
        shunt_gain = 0b01;  // 80mV
    } else if (shunt_max_voltage * multiplier <= 0.08f) {
        shunt_gain = 0b10;  // 160mV
    } else {
        if (int(shunt_max_voltage * multiplier * 100) > 16) {
            ESP_LOGW(TAG,
                    "    Max voltage across shunt resistor (resistance*current) exceeds %dmV. "
                    "This could damage the sensor!",
                    int(160 / multiplier));
        }
        shunt_gain = 0b11;  // 320mV
    }

    *config |= shunt_gain << 11;
    ESP_LOGCONFIG(TAG, "    Using %dV-Range Shunt Gain=%dmV", bus_32v_range ? 32 : 16, 40 << shunt_gain);
    if (!dev.write_byte_16(INA219_REGISTER_CONFIG, *config)) {
        return CALC_FAIL;
    }

    auto min_lsb = uint32_t(ceilf(this->max_current_a_ * 1000000.0f / 0x8000));
    auto max_lsb = uint32_t(floorf(this->max_current_a_ * 1000000.0f / 0x1000));
    uint32_t lsb = min_lsb;
    for (; lsb <= max_lsb; lsb++) {
        float max_current_before_overflow = lsb * 0x7FFF / 1000000.0f;
        if (this->max_current_a_ <= max_current_before_overflow)
            break;
    }
    if (lsb > max_lsb) {
        lsb = max_lsb;
        ESP_LOGW(TAG, "    The requested current (%0.02fA) cannot be achieved without an overflow", this->max_current_a_);
        return CALC_FAIL;
    }

    this->calibration_lsb_ = lsb;
    auto calibration = uint32_t(0.04096f / (0.000001 * lsb * this->shunt_resistance_ohm_));
    ESP_LOGV(TAG, "    Using LSB=%" PRIu32 " calibration=%" PRIu32, lsb, calibration);
    if (!dev.write_byte_16(INA219_REGISTER_CALIBRATION, calibration)) {
        return CALC_FAIL;
    }
    return CALC_OK;
}

void WindowController::setup() {
    // setup hardware
    this->boardId = 0xFF;
    if ((this->boardid0_pin_ == nullptr) || 
        (this->boardid1_pin_ == nullptr) || 
        (this->boardid2_pin_ == nullptr)) {
        this->mark_failed();
        this->faults |= WINCTRLFAULT_BOARDID_PIN_NULL;
        return;
    }
    this->boardid0_pin_->setup();
    this->boardid0_pin_->pin_mode(gpio::FLAG_INPUT | gpio::FLAG_PULLDOWN);
    this->boardid1_pin_->setup();
    this->boardid1_pin_->pin_mode(gpio::FLAG_INPUT | gpio::FLAG_PULLDOWN);
    this->boardid2_pin_->setup();
    this->boardid2_pin_->pin_mode(gpio::FLAG_INPUT | gpio::FLAG_PULLDOWN);
    this->boardId = (this->boardid2_pin_->digital_read() << 2) |
                    (this->boardid1_pin_->digital_read() << 1) |
                    (this->boardid0_pin_->digital_read() << 0);

    uint16_t config = 0x0000;
    if (CALC_FAIL == this->calcINA219config(&config, this->motA_ina219)) {
        this->mark_failed();
        return;
    }
    if (CALC_FAIL == this->calcINA219config(&config, this->motB_ina219)) {
        this->mark_failed();
        return;
    }
    delay(1);

    // setup MotorA
    if (ASSIGN_FAIL == motA.assignMotorPins(this->mota_enca_pin_, 
            this->mota_encb_pin_, this->mota_pwm_pin_, this->mota_in1_pin_,
            this->mota_in2_pin_)) {
        this->mark_failed();
        this->faults |= WINCTRLFAULT_BOARDID_PIN_NULL;
        return;
    }

    // setup MotorB
    if (ASSIGN_FAIL == motB.assignMotorPins(this->motb_enca_pin_, 
            this->motb_encb_pin_, this->motb_pwm_pin_, this->motb_in1_pin_,
            this->motb_in2_pin_)) {
        this->mark_failed();
        this->faults |= WINCTRLFAULT_BOARDID_PIN_NULL;
        return;
    }
}

//void ExampleComponent::loop() {
  // Tasks here will be performed at every call of the main application loop.
  // Note: code here MUST NOT BLOCK (see below)
  // Called every 7ms
  // Component Loop Control:
  // https://developers.esphome.io/architecture/components/advanced/#component-loop-control
//}

void WindowController::update() {
    this->boardId = (this->boardid2_pin_->digital_read() << 2) |
                    (this->boardid1_pin_->digital_read() << 1) |
                    (this->boardid0_pin_->digital_read() << 0);
    ESP_LOGI(TAG, " boardid: %d", this->boardId);

    uint16_t raw_bus_voltage;
    float bus_voltage_v;
    if (!this->motA_ina219.read_byte_16(INA219_REGISTER_BUS_VOLTAGE, &raw_bus_voltage)) {
      this->status_set_warning();
      return;
    }
    raw_bus_voltage >>= 3;
    bus_voltage_v = int16_t(raw_bus_voltage) * 0.004f;
    ESP_LOGI(TAG, " A bus_voltage: %f V", bus_voltage_v);

    if (!this->motB_ina219.read_byte_16(INA219_REGISTER_BUS_VOLTAGE, &raw_bus_voltage)) {
      this->status_set_warning();
      return;
    }
    raw_bus_voltage >>= 3;
    bus_voltage_v = int16_t(raw_bus_voltage) * 0.004f;
    ESP_LOGI(TAG, " B bus_voltage: %f V", bus_voltage_v);

    uint16_t raw_shunt_voltage;
    float shunt_voltage_mv;
    if (!this->motA_ina219.read_byte_16(INA219_REGISTER_SHUNT_VOLTAGE, &raw_shunt_voltage)) {
      this->status_set_warning();
      return;
    }
    shunt_voltage_mv = int16_t(raw_shunt_voltage) * 0.01f;
    ESP_LOGI(TAG, " A shunt_voltage: %f mV", shunt_voltage_mv);

    if (!this->motB_ina219.read_byte_16(INA219_REGISTER_SHUNT_VOLTAGE, &raw_shunt_voltage)) {
      this->status_set_warning();
      return;
    }
    shunt_voltage_mv = int16_t(raw_shunt_voltage) * 0.01f;
    ESP_LOGI(TAG, " B shunt_voltage: %f mV", shunt_voltage_mv);

    uint16_t raw_current;
    float current_ma;
    if (!this->motA_ina219.read_byte_16(INA219_REGISTER_CURRENT, &raw_current)) {
      this->status_set_warning();
      return;
    }
    current_ma = int16_t(raw_current) * (this->calibration_lsb_ / 1000.0f);
    ESP_LOGI(TAG, " A current: %2.2f A", current_ma/1000.0f);

    if (!this->motB_ina219.read_byte_16(INA219_REGISTER_CURRENT, &raw_current)) {
      this->status_set_warning();
      return;
    }
    current_ma = int16_t(raw_current) * (this->calibration_lsb_ / 1000.0f);
    ESP_LOGI(TAG, " B current: %2.2f A", current_ma/1000.0f);
}

uint8_t WindowController::getBoardId() const { return this->boardId; }

// Called once after booting and then each time a new client connects
//   to monitor logs
void WindowController::dump_config() {
    ESP_LOGCONFIG(TAG, "WindowController:");
    LOG_I2C_DEVICE(this);

    if (this->is_failed()) {
        ESP_LOGE(TAG, ESP_LOG_MSG_COMM_FAIL);
        return;
    }
    LOG_UPDATE_INTERVAL(this);
    LOG_PIN("  boardId0_pin: ", this->boardid0_pin_);
    LOG_PIN("  boardId1_pin: ", this->boardid1_pin_);
    LOG_PIN("  boardId2_pin: ", this->boardid2_pin_);
    LOG_PIN("  motaenca_pin: ", this->mota_enca_pin_);
    LOG_PIN("  motaencb_pin: ", this->mota_encb_pin_);
    LOG_PIN("  motapwm_pin: ", this->mota_pwm_pin_);
    LOG_PIN("  motain1_pin: ", this->mota_in1_pin_);
    LOG_PIN("  motain2_pin: ", this->mota_in2_pin_);
    LOG_PIN("  motbenca_pin: ", this->motb_enca_pin_);
    LOG_PIN("  motbencb_pin: ", this->motb_encb_pin_);
    LOG_PIN("  motbpwm_pin: ", this->motb_pwm_pin_);
    LOG_PIN("  motbin1_pin: ", this->motb_in1_pin_);
    LOG_PIN("  motbin2_pin: ", this->motb_in2_pin_);
    ESP_LOGCONFIG(TAG, "  boardId: %d", this->boardId);
}

// float ExampleComponent::get_setup_priority() const {
//   // Return the setup priority of this component
//   // Higher values mean this component will be set up later
//   return setup_priority::DATA;
// }

// void ExampleComponent::on_safe_shutdown() {
//   // Optional: Critical cleanup operations for safe shutdowns only
//   // This is called first, before any other shutdown procedures
//   ESP_LOGI(TAG, "Safe shutdown initiated");
// }

// void ExampleComponent::on_shutdown() {
//   // Optional: Start shutdown process
//   // For example, send a disconnect message but don't close connections yet
//   ESP_LOGI(TAG, "Starting shutdown");
// }

void WindowController::on_powerdown() {
  // Mode = 0 -> power down
  if (!this->write_byte_16(INA219_REGISTER_CONFIG, 0)) {
    ESP_LOGE(TAG, "powerdown error");
  }
}

// bool ExampleComponent::teardown() {
//   // Optional: Finish any pending operations
//   // Return false if more time is needed, true when done
//   // This will be called multiple times until it returns true or timeout is reached

//   // Note: Log messages here will likely only go to serial console
//   // as network connections are being closed. Avoid excessive logging
//   // to prevent slowing down the shutdown process.
//   return true;
// }

}  // namespace esphome::window_controller