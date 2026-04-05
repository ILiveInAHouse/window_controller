#include "esphome/core/log.h"
#include "windowController.h"

namespace esphome::window_controller {

static const char *TAG = "window_controller.component";

WindowController::WindowController() {
    // Constructor
    // Initialize class fields and configurations
}

void WindowController::setup() {
    // Initialize hardware
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