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
  mota_ina219_address: 0x40
  motb_ina219_address: 0x41
  address: 0x20 # required to make schema happy, but not used
  mota_enca_pin: GPIO14
  mota_encb_pin: GPIO4
  mota_pwm_pin: GPIO5
  mota_in1_pin: GPIO35
  mota_in2_pin: GPIO36
  motb_enca_pin: GPIO17
  motb_encb_pin: GPIO7
  motb_pwm_pin: GPIO6
  motb_in1_pin: GPIO37
  motb_in2_pin: GPIO38
  id: window_controller_hub_id

sensor:
  - platform: window_controller
    which_motor: MOTOR_A
    window_number:
      device_id: motor_A_device
      name: "Window Number"
    faults:
      device_id: motor_A_device
      name: "Faults"
  - platform: window_controller
    which_motor: MOTOR_B
    window_number:
      device_id: motor_B_device
      name: "Window Number"
    faults:
      device_id: motor_B_device
      name: "Faults"

number:
  - platform: window_controller
    window_controller_id: window_controller_hub_id
    name: "Hub Position"
    unit_of_measurement: "%"
    icon: "mdi:gauge"
    mode: SLIDER
```