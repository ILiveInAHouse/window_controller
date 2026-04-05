# windowController
Motorized windows: controller running 1 or 2 windowMotors

```yaml
# example configuration:

external_components:
  - source:
      type: local
      path: ../components

windowController:
  boardid0_pin: GPIO18
  boardid1_pin: GPIO43
  boardid2_pin: GPIO33
```