#include "esphome/core/log.h"
#include "windowController.h"

namespace esphome::window_controller {

static const char *TAG = "window_controller.component";

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
}

uint8_t WindowController::getBoardId() const { return this->boardId; }

// Called once after booting and then each time a new client connects
//   to monitor logs
void WindowController::dump_config() {
    ESP_LOGCONFIG(TAG, "WindowController:");
    LOG_PIN("  boardId0_pin: ", this->boardid0_pin_);
    LOG_PIN("  boardId1_pin: ", this->boardid1_pin_);
    LOG_PIN("  boardId2_pin: ", this->boardid2_pin_);
    LOG_PIN("  scl_pin: ", this->scl_pin_);
    LOG_PIN("  sda_pin: ", this->sda_pin_);
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