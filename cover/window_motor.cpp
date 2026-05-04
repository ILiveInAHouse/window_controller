#include "window_motor.h"

#include "esphome/core/log.h"

namespace esphome {
namespace window_controller {

static const char *const TAG = "windowmotor";

void WindowMotorClass::setup() {
   this->setup_called = true;
}

void WindowMotorClass::update() {
   // Called at WindowMotorClass polling rate
   ESP_LOGI(TAG, "motor=%c update", (this->whichMotor == MOTOR_A) ? 'A' : 'B');
}

void WindowMotorClass::calcWinNumAndStsMsk() {
    if (this->ui->boardId > MAX_BOARD_ID) {
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
    this->ui->window_number->publish_state(windowNumber);
    this->windowNumber = windowNumber;
    this->statusMask = 2 << (((this->ui->boardId * 2) + ((this->whichMotor == MOTOR_A) ? 0 : 1)));
}

WhichMotorEnum WindowMotorClass::getWhichMotor() {
   return this->whichMotor;
}

void WindowMotorClass::child_setup(WCMotorUI *ui) {
   this->boardid = boardid;
   this->ui = ui;
   this->calcWinNumAndStsMsk();
}

void WindowMotorClass::child_publish_info() {
}

// Called once after booting and then each time a new client connects
//   to monitor logs
void WindowMotorClass::child_dump_config() {
    LOG_PIN("  enca_pin: ", this->enca_pin_);
    LOG_PIN("  encb_pin: ", this->encb_pin_);
    LOG_PIN("  pwm_pin: ", this->pwm_pin_);
    LOG_PIN("  in1_pin: ", this->in1_pin_);
    LOG_PIN("  in2_pin: ", this->in2_pin_);

}

void WindowMotorClass::child_update() {
   // Called from parent hub.
   // Could do some synch work here.
   ESP_LOGI(TAG, "Ymotor=%c child_update setup_called=%d winnum=%d", (this->whichMotor == MOTOR_A) ? 'A' : 'B', this->setup_called, this->windowNumber);
}

} // namespace window_controller
} // namespace esphome
