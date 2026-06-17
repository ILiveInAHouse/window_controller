#include "window_motor.h"

#include "esphome/core/log.h"

namespace esphome {
namespace window_controller {

static const char *const TAG = "windowmotor";

// Note: ac_dimmer component has GPIO interrupts and PWM output

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
   // Saleae says the i2c read takes about 2.268 ms
    uint16_t raw_current;
    if (!this->ina219.read_byte_16(INA219_REGISTER_CURRENT, &raw_current)) {
      //this->ina219.status_set_warning();
      this->setFault(MOTFAULT_INA219_READ);
      return FUNC_FAIL;
    }
    *current_a = int16_t(raw_current) * (this->calibration_lsb_ / 1000.0f) / 1000.0f;
    // Current will be negative in one motor direction, positive in the other
    *current_a = fabsf(*current_a);
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

//
// Control callbacks
//
void WindowMotorClass::controlTargetPosition(float value) {
   ESP_LOGD("custom", "controlTargetPosition=%f which=%d", value, this->whichMotor);
}
void WindowMotorClass::controlAllMotorStatus(float value) {
   ESP_LOGD("custom", "controlAllMotorStatus=%f which=%d", value, this->whichMotor);
}

//
// Encoder callback
//
void WindowMotorClass::encoderListener(int32_t stepval) {
   // micros() returns usec since reset.  It will roll over ~ every 71 minutes.
   uint32_t now = micros();
   if ((this->motmode == MOTMODE_STOP) || 
       (this->motmode == MOTMODE_SHORTBRAKE) || 
       (this->encoderLastCallback_us == INVALID_ENCODER_LAST_CALLBACK_US)) {
      this->encoderSpeed_stepspers = 0.0f;
   } else {
      this->encoderSpeed_stepspers = ((float)(stepval) - this->encoderLastCounter) / 
         ((float)(now - this->encoderLastCallback_us) / 1000000);
   }
   if (stepval > this->encoderCounterAtOpen) {
      this->encoderCounterAtOpen = stepval;
   }
   if (stepval < this->encoderCounterAtClosed) {
      this->encoderCounterAtClosed = stepval;
   }
   this->encoderLastCounter = stepval;
   if ((this->motmode == MOTMODE_STOP) || (this->motmode == MOTMODE_SHORTBRAKE)) {
      this->encoderLastCallback_us = INVALID_ENCODER_LAST_CALLBACK_US;
   } else {
      this->encoderLastCallback_us = now;
   }
}

//
// Pin control
//
bool WindowMotorClass::setup_pins() {
   if ((this->in1_pin_ == nullptr) || 
       (this->in2_pin_ == nullptr)) {
      return FUNC_FAIL;
    }

    this->in1_pin_->setup();
    this->in1_pin_->pin_mode(gpio::FLAG_OUTPUT);
    this->in2_pin_->setup();
    this->in2_pin_->pin_mode(gpio::FLAG_OUTPUT);
   //  this->enca_pin_->setup();
   //  this->enca_pin_->pin_mode(gpio::FLAG_INPUT | gpio::FLAG_PULLDOWN);
   //  this->encb_pin_->setup();
   //  this->encb_pin_->pin_mode(gpio::FLAG_INPUT | gpio::FLAG_PULLDOWN);

    return FUNC_OK;
}

void WindowMotorClass::setMotorDriverMode(MotorDriverModeEnum mode) {
   this->motmode = mode;
   switch(mode) {
      case MOTMODE_CW:
         this->in1_pin_->digital_write(true);
         this->in2_pin_->digital_write(false);
         break;
      case MOTMODE_CCW:
         this->in1_pin_->digital_write(false);
         this->in2_pin_->digital_write(true);
         break;
      case MOTMODE_SHORTBRAKE:
         this->in1_pin_->digital_write(true);
         this->in2_pin_->digital_write(true);
         break;
      case MOTMODE_STOP:
         this->in1_pin_->digital_write(false);
         this->in2_pin_->digital_write(false);
         break;
   }
}

void WindowMotorClass::setWindowDirection(WindowDirectionEnum dir) {
   this->windir = dir;
   switch(dir) {
      case WINDIR_OPEN:
         this->setMotorDriverMode(MOTMODE_CW);
         break;
      case WINDIR_CLOSE:
         this->setMotorDriverMode(MOTMODE_CCW);
         break;
      case WINDIR_STOP:
         this->setMotorDriverMode(MOTMODE_STOP);
         break;
   }
}

//
// Class functions
//
WindowMotorClass::WindowMotorClass() {
    // Constructor
    // Initialize class fields and configurations
}

void WindowMotorClass::setFault(uint32_t fault_bit) {
   this->faults |= fault_bit;
   this->ui->faults_Sensor->publish_state(this->faults);
}

void WindowMotorClass::setMotorStatus(uint16_t sts) {
   this->ui->motor_status_Sensor->publish_state(sts);
}

void WindowMotorClass::setEstPosition(float pos) {
   //this->ui->est_position_Sensor->set_state_value(pos);
   this->ui->est_position_Sensor->publish_state(pos);
}

#define PWM_MAX 1.0f
#define PWM_STEP 1.0f
void WindowMotorClass::runPwm() {
   if (this->duty < PWM_MAX) {
      this->duty += PWM_STEP;
      this->duty = clamp(this->duty, 0.0f, PWM_MAX);
   }
   this->ui->pwm_FloatOutput->set_level(this->duty);
   ESP_LOGI(TAG, " which=%c duty=%1.2f", (this->whichMotor==MOTOR_A) ? 'A' : 'B', this->duty);
}

void WindowMotorClass::stopMotor() {
   this->setWindowDirection(WINDIR_STOP);
   this->ui->pwm_FloatOutput->set_level(0.0f);
   this->duty = 0.0f;
   this->encoderLastCallback_us = INVALID_ENCODER_LAST_CALLBACK_US;
   this->encoderSpeed_stepspers = 0.0f;
}

void WindowMotorClass::setup() {
   // this->ui may not be available when this runs
}

void WindowMotorClass::update() {
   this->pollMotorMove();
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

#define DEFAULT_MAX_TORQUE 0.5f
void WindowMotorClass::child_setup(WCMotorUI *ui) {
   if (ui == nullptr) {
      this->mark_failed();
      return;
   }
   this->ui = ui;
   this->setFault(0x0);
   this->setMotorStatus(0x0);
   this->setEstPosition(0.0f);
   this->stopMotor();

   this->ui->max_torque_Number->publish_state(DEFAULT_MAX_TORQUE);
   // Set up i2c
   if (FUNC_FAIL == this->calcINA219config()) {
      this->setFault(MOTFAULT_INA219_INIT);
      this->mark_failed();
      return;
   }

   // Calculate window number and statusMask
   this->calcWinNumAndStsMsk();

   // Set up control callbacks
   this->ui->target_position_Number->add_on_state_callback([this](float value) {
      this->controlTargetPosition(value);
   });
   this->ui->all_motor_status_Number->add_on_state_callback([this](float value) {
      this->controlAllMotorStatus(value);
   });
   // Set up rotary_encoder callback
   this->ui->enc_RotaryEncoderSensor->register_listener([this](int32_t value) {
      this->encoderListener(value);
   });

   // Set up pins
   if (this->setup_pins() == FUNC_FAIL) {
      this->setFault(MOTFAULT_PIN_INIT);
      this->mark_failed();
      return;
   }

   this->setWindowDirection(WINDIR_STOP);

}

void WindowMotorClass::child_publish_info() {
}

bool WindowMotorClass::my_turn_to_move() {
   // if no window_number less than mine have work to do, then
   //   it's ok for me to move my window
   // motor status is a bit mask.  each window has a bit in order:
   // 0x0001 (all windows stop)
   // 0x0002 (window 1)
   // 0x0004 (window 2)
   // 0x0008 (window 5)
   // 0x0010 (window 6)
   // 0x0020 (window 9)
   // 0x0040 (window 10)
   // 0x0080 (window 11)
   // 0x0100 (window 12)
   // 0x0200 (window 13)
   // 0x0400 (window 14)
   // all_motor_status comes from Hass
   if (((int)(this->ui->all_motor_status_Number->state) & (this->statusMask-1)) > 0) {
      return false;
   }
   if ((this->ui->co_motor_status & (this->statusMask-1)) > 0) {
      return false;
   }
   return true;
}

// Called once after booting and then each time a new client connects
//   to monitor logs
void WindowMotorClass::dump_config() {
    LOG_I2C_DEVICE(this);
   //  LOG_PIN("  enca_pin: ", this->enca_pin_);
   //  LOG_PIN("  encb_pin: ", this->encb_pin_);
    LOG_PIN("  in1_pin: ", this->in1_pin_);
    LOG_PIN("  in2_pin: ", this->in2_pin_);

}

#define EST_POS_STEP 1.0f
void WindowMotorClass::pollMotorMove() {
   float tar = this->ui->target_position_Number->state;
   float est = this->ui->est_position_Sensor->get_state();
   if (!isEqual(tar, est, 0.99f)) {
      this->setMotorStatus(this->statusMask);
      this->ui->parent->set_co_motor_status_mask(this->statusMask);
      if (this->my_turn_to_move()) {
         // No other motors below me have work to do, so it's ok for me to do work.
         float current_a;
         this->getCurrent(&current_a);
         if (current_a > this->largest_current_ever_a) {
            this->largest_current_ever_a = current_a;
         }
         ESP_LOGI(TAG, " %c current=%2.2f target_pos=%3.2f est_pos=%3.2f speed=%3.4f enc=%d",
            (this->whichMotor==MOTOR_A) ? 'A' : 'B', current_a, tar, est, this->encoderSpeed_stepspers, this->encoderLastCounter);
         if (tar < est) {
            this->setWindowDirection(WINDIR_CLOSE);
            this->runPwm();
            est = est - std::min(EST_POS_STEP, est - tar);
         } else {
            this->setWindowDirection(WINDIR_OPEN);
            this->runPwm();
            est = est + std::min(EST_POS_STEP, tar - est);
         }
         est = clamp(est, 0.0f, 100.0f);
         if (current_a > this->ui->max_torque_Number->state) {
            // possibly at the end of our journey
            ESP_LOGI(TAG, "max current hit %2.3f", current_a);
            est = tar;
         }
         this->setEstPosition(est);
         if (isEqual(tar, est, 0.02f)) {
            this->stopMotor();
            this->setMotorStatus(0);
            this->ui->parent->clear_co_motor_status_mask(this->statusMask);
         }
      } else {
         // another lower motor has work to do so stop my motor
         this->stopMotor();
      }
   } // if !eq(tar,est)
}

void WindowMotorClass::child_sync_update() {
   // Called from parent hub.
   // Could do some synch work here.
   // Called at WindowMotorClass polling rate
   float bus_voltage_v;
   this->getBusVoltage(&bus_voltage_v);
   ESP_LOGI(TAG, " %c biggestI=%2.3fA",
          (this->whichMotor==MOTOR_A) ? 'A' : 'B', this->largest_current_ever_a);
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
