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
    this->boardId = 0xff;
    this->windowNumber = 0;
    this->statusMask = 0x0;
}

#define FUNC_OK 1
#define FUNC_FAIL 0
bool WindowMotor::assignMotorPins(InternalGPIOPin *enca_pin, 
                        InternalGPIOPin *encb_pin, InternalGPIOPin *pwm_pin, 
                        InternalGPIOPin *in1_pin, InternalGPIOPin *in2_pin) {
    if ((enca_pin == nullptr) || 
        (encb_pin == nullptr) || 
        (pwm_pin == nullptr) || 
        (in1_pin == nullptr) || 
        (in2_pin == nullptr)) {
        return FUNC_FAIL;
    }
    this->encA_pin_ = enca_pin;
    this->encB_pin_ = encb_pin;
    this->pwm_pin_ = pwm_pin;
    this->in1_pin_ = in1_pin;
    this->in2_pin_ = in2_pin;
    return FUNC_OK;
}

bool WindowMotor::calcINA219config() {
    uint16_t config = 0x0000;
    // INA219 config
    // Bus ADC and Shunt ADC 12 bit+128 samples
    config = 0x0000;
    // Continuous operation of Bus and Shunt ADCs
    config |= 0b0000000000000111;
    // Bus ADC and Shunt ADC 12 bit+128 samples -> 68.10 ms
    config |= 0b0000011110000000;
    config |= 0b0000000001111000;
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
        config |= 0b0010000000000000;
        multiplier = 0.5f;
    } else {
        config |= 0b0000000000000000;
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

    config |= shunt_gain << 11;
    ESP_LOGCONFIG(TAG, "    Using %dV-Range Shunt Gain=%dmV", bus_32v_range ? 32 : 16, 40 << shunt_gain);
    if (!this->ina219.write_byte_16(INA219_REGISTER_CONFIG, config)) {
        return FUNC_FAIL;
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
        return FUNC_FAIL;
    }

    this->calibration_lsb_ = lsb;
    auto calibration = uint32_t(0.04096f / (0.000001 * lsb * this->shunt_resistance_ohm_));
    ESP_LOGV(TAG, "    Using LSB=%" PRIu32 " calibration=%" PRIu32, lsb, calibration);
    if (!this->ina219.write_byte_16(INA219_REGISTER_CALIBRATION, calibration)) {
        return FUNC_FAIL;
    }
    return FUNC_OK;
}

bool WindowMotor::getBusVoltage(float *bus_voltage_v) {
    uint16_t raw_bus_voltage;
    if (!this->ina219.read_byte_16(INA219_REGISTER_BUS_VOLTAGE, &raw_bus_voltage)) {
      //this->ina219.status_set_warning();
      return FUNC_FAIL;
    }
    raw_bus_voltage >>= 3;
    *bus_voltage_v = int16_t(raw_bus_voltage) * 0.004f;
    return FUNC_OK;
}

bool WindowMotor::getCurrent(float *current_a) {
    uint16_t raw_current;
    if (!this->ina219.read_byte_16(INA219_REGISTER_CURRENT, &raw_current)) {
      //this->ina219.status_set_warning();
      return FUNC_FAIL;
    }
    *current_a = int16_t(raw_current) * (this->calibration_lsb_ / 1000.0f) / 1000.0f;
    return FUNC_OK;
}

bool WindowMotor::getShuntVoltage(float *shunt_voltage_mv) {
    uint16_t raw_shunt_voltage;
    if (!this->ina219.read_byte_16(INA219_REGISTER_SHUNT_VOLTAGE, &raw_shunt_voltage)) {
      //this->ina219.status_set_warning();
      return FUNC_FAIL;
    }
    *shunt_voltage_mv = int16_t(raw_shunt_voltage) * 0.01f;
    return FUNC_OK;
}

bool WindowMotor::powerdownINA219() {
  // Mode = 0 -> power down
  if (!this->ina219.write_byte_16(INA219_REGISTER_CONFIG, 0)) {
    ESP_LOGE(TAG, "powerdown error");
  }
  return FUNC_OK;
}

void WindowMotor::calcWinNumAndStsMsk() {
    if (this->boardId > MAX_BOARD_ID) {
        return;
    }
    // Calculate Window number and status mask
    //                           statusMask 0x0001 reserved for all-stop
    // boardId 0 = windows 1&2   statusMask 0x0002 & 0x0004
    // boardId 1 = windows 5&6   statusMask 0x0008 & 0x0010
    // boardId 2 = windows 9&10  statusMask 0x0020 & 0x0040
    // boardId 3 = windows 11&12 statusMask 0x0080 & 0x0100
    // boardId 4 = windows 13&14 statusMask 0x0200 & 0x0400
    this->windowNumber = 1;
    if (!this->isMotorA) {
        this->windowNumber++;
    }
    if (this->boardId > 0) {
        this->windowNumber += 4;
    }
    if (this->boardId > 1) {
        this->windowNumber += 4;
        this->windowNumber += (2*(this->boardId-2));
    }
    this->statusMask = 2 << (((this->boardId * 2) + ((this->isMotorA) ? 0 : 1)));
}

bool WindowMotor::setup(uint8_t boardId, bool isMotorA, InternalGPIOPin *enca_pin, 
                        InternalGPIOPin *encb_pin, InternalGPIOPin *pwm_pin, 
                        InternalGPIOPin *in1_pin, InternalGPIOPin *in2_pin) {
    if (FUNC_FAIL == this->calcINA219config()) {
        return FUNC_FAIL;
    }
    if (FUNC_FAIL == this->assignMotorPins(enca_pin, encb_pin, pwm_pin, 
            in1_pin, in2_pin)) {
        this->faults |= WINMOTFAULT_PIN_NULL;
        return FUNC_FAIL;
    }
    this->isMotorA = isMotorA;
    this->boardId = boardId;
    this->calcWinNumAndStsMsk();
    return FUNC_OK;
}

void WindowMotor::update() {
    this->calcWinNumAndStsMsk();
    float bus_voltage_v;
    this->getBusVoltage(&bus_voltage_v);
    float current_a;
    this->getCurrent(&current_a);
    ESP_LOGI(TAG, " %c win#=%d stsMsk=%04x bus_voltage=%2.2fV current:%2.2fA", 
            (this->isMotorA) ? 'A' : 'B', this->windowNumber, this->statusMask, 
            bus_voltage_v, current_a);
}

WindowController::WindowController() {
    // Constructor
    // Initialize class fields and configurations
    this->shutdownImminent = false;
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
    if (boardId > MAX_BOARD_ID) {
        this->mark_failed();
        return;
    }

    // setup MotorA
    if (FUNC_FAIL == this->motA.setup(this->boardId, true, this->mota_enca_pin_, 
            this->mota_encb_pin_, this->mota_pwm_pin_, this->mota_in1_pin_,
            this->mota_in2_pin_)) {
        this->mark_failed();
        return;
    }
    // setup MotorB
    if (FUNC_FAIL == this->motB.setup(this->boardId, false, this->mota_enca_pin_, 
            this->mota_encb_pin_, this->mota_pwm_pin_, this->mota_in1_pin_,
            this->mota_in2_pin_)) {
        this->mark_failed();
        return;
    }
    delay(1);
}

//void ExampleComponent::loop() {
  // Tasks here will be performed at every call of the main application loop.
  // Note: code here MUST NOT BLOCK (see below)
  // Called every 7ms
  // Component Loop Control:
  // https://developers.esphome.io/architecture/components/advanced/#component-loop-control
//}

void WindowController::update() {
    if (this->shutdownImminent) {
        return;
    }
    this->boardId = (this->boardid2_pin_->digital_read() << 2) |
                    (this->boardid1_pin_->digital_read() << 1) |
                    (this->boardid0_pin_->digital_read() << 0);
    ESP_LOGI(TAG, " boardid: %d", this->boardId);

    // update Motor A
    this->motA.boardId = this->boardId;
    this->motA.update();

    // update Motor B
    this->motB.boardId = this->boardId;
    this->motB.update();
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

void WindowController::on_safe_shutdown() {
  // Optional: Critical cleanup operations for safe shutdowns only
  // This is called first, before any other shutdown procedures
  // ESP_LOGI(TAG, "Safe shutdown initiated");
  this->motA.powerdownINA219();
  this->motB.powerdownINA219();
  this->shutdownImminent = true;
}

void WindowController::on_shutdown() {
  // Optional: Start shutdown process
  // For example, send a disconnect message but don't close connections yet
  // ESP_LOGI(TAG, "Starting shutdown");
  this->shutdownImminent = true;
}

// void WindowController::on_powerdown() {
// }

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