# windowController
Motorized windows: controller running 1 or 2 windowMotors

```yaml
# example configuration:
esphome:
  name: windowctrl-dev
  friendly_name: windowCtrl_dev

  devices:
    - id: motor_A_device
      name: "MotorA"
    - id: motor_B_device
      name: "MotorB"

i2c:
  sda: GPIO8
  scl: GPIO9
  frequency: 400kHz

external_components:
  - source:
      type: local
      path: ../components

window_controller:
  boardid0_pin: GPIO18
  boardid1_pin: GPIO43
  boardid2_pin: GPIO33
  id: window_controller_hub_id

sensor:
  - platform: window_controller
    window_controller_id: window_controller_hub_id
    which_motor: MOTOR_A
    window_number:
      device_id: motor_A_device
      name: "Window Number"
    faults:
      device_id: motor_A_device
      name: "Faults"
    motor_status:
      device_id: motor_A_device
      name: "Motor Status"
    est_position:
      device_id: motor_A_device
      name: "Est Position"
  - platform: window_controller
    window_controller_id: window_controller_hub_id
    which_motor: MOTOR_B
    window_number:
      device_id: motor_B_device
      name: "Window Number"
    faults:
      device_id: motor_B_device
      name: "Faults"
    motor_status:
      device_id: motor_B_device
      name: "Motor Status"
    est_position:
      device_id: motor_B_device
      name: "Est Position"

cover:
  - platform: window_controller
    window_controller_id: window_controller_hub_id
    # device_class: window
    which_motor: MOTOR_A
    enca_pin: GPIO14
    encb_pin: GPIO4
    pwm_pin: GPIO5
    in1_pin: GPIO35
    in2_pin: GPIO36
    ina219_address: 0x40
    address: 0x20 # required to make schema happy, but not used
  - platform: window_controller
    window_controller_id: window_controller_hub_id
    # device_class: window
    which_motor: MOTOR_B
    enca_pin: GPIO17
    encb_pin: GPIO7
    pwm_pin: GPIO6
    in1_pin: GPIO37
    in2_pin: GPIO38
    ina219_address: 0x41
    address: 0x20 # required to make schema happy, but not used

number:
  - platform: window_controller
    window_controller_id: window_controller_hub_id
    target_position:
      name: "Target Position"
      which_motor: MOTOR_A
      device_id: motor_A_device
    all_motor_status:
      name: "All Motor Status"
      which_motor: MOTOR_A
      device_id: motor_A_device
      mode: BOX
    max_torque:
      name: "Max Torque"
      which_motor: MOTOR_A
      device_id: motor_A_device
  - platform: window_controller
    window_controller_id: window_controller_hub_id
    target_position:
      name: "Target Position"
      which_motor: MOTOR_B
      device_id: motor_B_device
    all_motor_status:
      name: "All Motor Status"
      which_motor: MOTOR_B
      device_id: motor_B_device
      mode: BOX
    max_torque:
      name: "Max Torque"
      which_motor: MOTOR_B
      device_id: motor_B_device
```