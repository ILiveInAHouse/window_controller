#include "window_motor.h"

#include "esphome/core/log.h"

namespace esphome {
namespace window_controller {

static const char *const TAG = "windowmotor";

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
bool WindowMotorClass::calcINA219config() {
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

bool WindowMotorClass::getBusVoltage(float *bus_voltage_v) {
    uint16_t raw_bus_voltage;
    if (!this->ina219.read_byte_16(INA219_REGISTER_BUS_VOLTAGE, &raw_bus_voltage)) {
      //this->ina219.status_set_warning();
      return FUNC_FAIL;
    }
    raw_bus_voltage >>= 3;
    *bus_voltage_v = int16_t(raw_bus_voltage) * 0.004f;
    return FUNC_OK;
}

bool WindowMotorClass::getCurrent(float *current_a) {
    uint16_t raw_current;
    if (!this->ina219.read_byte_16(INA219_REGISTER_CURRENT, &raw_current)) {
      //this->ina219.status_set_warning();
      return FUNC_FAIL;
    }
    *current_a = int16_t(raw_current) * (this->calibration_lsb_ / 1000.0f) / 1000.0f;
    return FUNC_OK;
}

bool WindowMotorClass::getShuntVoltage(float *shunt_voltage_mv) {
    uint16_t raw_shunt_voltage;
    if (!this->ina219.read_byte_16(INA219_REGISTER_SHUNT_VOLTAGE, &raw_shunt_voltage)) {
      //this->ina219.status_set_warning();
      return FUNC_FAIL;
    }
    *shunt_voltage_mv = int16_t(raw_shunt_voltage) * 0.01f;
    return FUNC_OK;
}

bool WindowMotorClass::powerdownINA219() {
  // Mode = 0 -> power down
  if (!this->ina219.write_byte_16(INA219_REGISTER_CONFIG, 0)) {
    ESP_LOGE(TAG, "powerdown error");
  }
  return FUNC_OK;
}

WindowMotorClass::WindowMotorClass() {
    // Constructor
    // Initialize class fields and configurations
}

void WindowMotorClass::setFault(uint32_t fault_bit) {
   this->faults |= fault_bit;
   this->ui->faults_Sensor->publish_state(this->faults);
}

void WindowMotorClass::setup() {
   // this->ui may not be available when this runs
}

void WindowMotorClass::update() {
}

void WindowMotorClass::calcWinNumAndStsMsk() {
    if (this->ui->boardId > MAX_BOARD_ID) {
      this->mark_failed();
      return;
    }
    // Calculate Window number and status mask
    //                           statusMask 0x0001 reserved for all-stop
    // boardId 0 = windows 1&2   statusMask 0x0002 & 0x0004
    // boardId 1 = windows 5&6   statusMask 0x0008 & 0x0010
    // boardId 2 = windows 9&10  statusMask 0x0020 & 0x0040
    // boardId 3 = windows 11&12 statusMask 0x0080 & 0x0100
    // boardId 4 = windows 13&14 statusMask 0x0200 & 0x0400
    int windowNumber = 1;
    if (this->whichMotor != MOTOR_A) {
        windowNumber++;
    }
    if (this->ui->boardId > 0) {
        windowNumber += 4;
    }
    if (this->ui->boardId > 1) {
        windowNumber += 4;
        windowNumber += (2*(this->ui->boardId-2));
    }
    this->ui->window_number_Sensor->publish_state(windowNumber);
    this->windowNumber = windowNumber;
    this->statusMask = 2 << (((this->ui->boardId * 2) + ((this->whichMotor == MOTOR_A) ? 0 : 1)));
}

void WindowMotorClass::child_setup(WCMotorUI *ui) {
   if (ui == nullptr) {
      this->mark_failed();
      return;
   }
   this->ui = ui;
   this->setFault(0x0);
   if (FUNC_FAIL == this->calcINA219config()) {
      this->setFault(MOTFAULT_INA219_INIT);
      this->mark_failed();
      return;
   }
   this->calcWinNumAndStsMsk();
}

void WindowMotorClass::child_publish_info() {
}

// Called once after booting and then each time a new client connects
//   to monitor logs
void WindowMotorClass::dump_config() {
    LOG_I2C_DEVICE(this);
    LOG_PIN("  enca_pin: ", this->enca_pin_);
    LOG_PIN("  encb_pin: ", this->encb_pin_);
    LOG_PIN("  pwm_pin: ", this->pwm_pin_);
    LOG_PIN("  in1_pin: ", this->in1_pin_);
    LOG_PIN("  in2_pin: ", this->in2_pin_);

}

void WindowMotorClass::child_sync_update() {
   // Called from parent hub.
   // Could do some synch work here.
   // Called at WindowMotorClass polling rate
   float bus_voltage_v;
   this->getBusVoltage(&bus_voltage_v);
   float current_a;
   this->getCurrent(&current_a);
   ESP_LOGI(TAG, " %c win#=%d stsMsk=0x%08x bus_voltage=%2.2fV current=%2.2fA",
         (this->whichMotor==MOTOR_A) ? 'A' : 'B', this->windowNumber, this->statusMask, 
         bus_voltage_v, current_a);
   // ESP_LOGI(TAG, "motor=%c child_sync_update winnum=%d", (this->whichMotor == MOTOR_A) ? 'A' : 'B', this->windowNumber);
}

void WindowMotorClass::on_safe_shutdown() {
  // Optional: Critical cleanup operations for safe shutdowns only
  // This is called first, before any other shutdown procedures
  // ESP_LOGI(TAG, "Safe shutdown initiated");
  this->shutdownImminent = true;
  this->powerdownINA219();
}

void WindowMotorClass::on_shutdown() {
  this->shutdownImminent = true;
}

} // namespace window_controller
} // namespace esphome
