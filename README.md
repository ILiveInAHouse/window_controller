# windowController
Motorized windows: controller running 1 or 2 windowMotors

```yaml
# example configuration:

i2c:
  sda: GPIO8
  scl: GPIO9

external_components:
  - source:
      type: local
      path: ../components

windowController:
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
```