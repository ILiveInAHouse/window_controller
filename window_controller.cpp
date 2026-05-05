#include "esphome/core/log.h"
#include "esphome/core/hal.h"
#include "window_controller.h"

namespace esphome::window_controller {

static const char *TAG = "window_controller_hub.component";

void WCNumber::control(float value) {
    this->publish_state(value);
    ESP_LOGD("custom", "Slider value changed to: %f which=%d", value, this->whichMotor);
    // You can also call a parent method here if you want
    if (this->parent_ != nullptr) {
        this->parent_->print_number_change(value);
    }
}

WindowMotor::WindowMotor() {
    this->faults = 0;
    this->boardId = 0xff;
    this->windowNumber = 0;
    this->statusMask = 0x0;
    this->targetPosition = 0;
}

void WindowMotor::setAllMotorStatus(uint16_t newsts) {
    this->allMotorStatus = newsts;
}

bool WindowMotor::getIsMotorA() {
    return this->isMotorA;
}

uint32_t WindowMotor::getFaults() {
    return this->faults;
}

bool WindowMotor::setup(uint8_t boardId, bool isMotorA) {
    this->isMotorA = isMotorA;
    this->boardId = boardId;
    //this->calcWinNumAndStsMsk(); // moved to window_motor.cpp
    return FUNC_OK;
}

void WindowMotor::update() {
    //this->calcWinNumAndStsMsk(); // moved to window_motor.cpp
    ESP_LOGI(TAG, " %c Xwin#=%d stsMsk=%04x", 
            (this->isMotorA) ? 'A' : 'B', this->windowNumber, this->statusMask);
}

WindowControllerHub::WindowControllerHub() {
    // Constructor
    // Initialize class fields and configurations
    this->shutdownImminent = false;
    this->motuiA.whichMotor = MOTOR_A;
    this->motuiB.whichMotor = MOTOR_B;
}

void WindowControllerHub::setup() {
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
    this->motuiA.boardId = this->boardId;
    this->motuiB.boardId = this->boardId;

    // setup MotorA
    if (FUNC_FAIL == this->motA.setup(this->boardId, true)) {
        this->mark_failed();
        return;
    }
    // setup MotorB
    if (FUNC_FAIL == this->motB.setup(this->boardId, false)) {
        this->mark_failed();
        return;
    }

    // each component type (sensor, fan, etc) is a new child
    // each component type's device_id specified in the .yaml is a new child
    for (auto *child : this->children_) {
        if (child->getWhichMotor() == MOTOR_A) {
            child->child_setup(&this->motuiA);
        }
        if (child->getWhichMotor() == MOTOR_B) {
            child->child_setup(&this->motuiB);
        }
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

void WindowControllerHub::update() {
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

    this->all_children_publish_info();
    this->all_children_update();
}

void WindowControllerHub::all_children_update() {
    // traditional hub architecture has each component type (sensor, fan, etc) is a new child
    // I'm using these children to reference each motor
    for (auto *child : this->children_) {
        child->child_update();
    }
}

void WindowControllerHub::all_children_publish_info() {
    // traditional hub architecture has each component type (sensor, fan, etc) is a new child
    // I'm using these children to reference each motor
    for (auto *child : this->children_) {
        child->child_publish_info();
    }
}

void WindowControllerHub::setAllMotorStatus(uint16_t newsts) {
    this->motA.setAllMotorStatus(newsts);
    this->motB.setAllMotorStatus(newsts);
}

uint8_t WindowControllerHub::getBoardId() const { return this->boardId; }

uint32_t WindowControllerHub::getFaults() {
    return this->motA.getFaults();
}

// Called once after booting and then each time a new client connects
//   to monitor logs
void WindowControllerHub::dump_config() {
    ESP_LOGCONFIG(TAG, "WindowControllerHub:");

    if (this->is_failed()) {
        ESP_LOGE(TAG, ESP_LOG_MSG_COMM_FAIL);
        return;
    }
    LOG_UPDATE_INTERVAL(this);
    LOG_PIN("  boardId0_pin: ", this->boardid0_pin_);
    LOG_PIN("  boardId1_pin: ", this->boardid1_pin_);
    LOG_PIN("  boardId2_pin: ", this->boardid2_pin_);
    ESP_LOGCONFIG(TAG, "  boardId: %d", this->boardId);
}

void WindowControllerHub::all_children_dump_config() {
    // traditional hub architecture has each component type (sensor, fan, etc) is a new child
    // I'm using these children to reference each motor
    for (auto *child : this->children_) {
        child->child_dump_config();
    }
}

// float ExampleComponent::get_setup_priority() const {
//   // Return the setup priority of this component
//   // Higher values mean this component will be set up later
//   return setup_priority::DATA;
// }

void WindowControllerHub::on_safe_shutdown() {
  // Optional: Critical cleanup operations for safe shutdowns only
  // This is called first, before any other shutdown procedures
  // ESP_LOGI(TAG, "Safe shutdown initiated");
  this->shutdownImminent = true;
    for (auto *child : this->children_) {
        child->child_on_safe_shutdown();
    }
}

void WindowControllerHub::on_shutdown() {
  // Optional: Start shutdown process
  // For example, send a disconnect message but don't close connections yet
  // ESP_LOGI(TAG, "Starting shutdown");
  this->shutdownImminent = true;
}

// void WindowControllerHub::on_powerdown() {
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

void WindowControllerHub::register_child(WindowControllerClient *obj) {
  this->children_.push_back(obj);
  obj->set_parent(this);
}

void WindowControllerHub::print_number_change(float new_n) {
    ESP_LOGI(TAG, "new_n=%f", new_n);
}

}  // namespace esphome::window_controller