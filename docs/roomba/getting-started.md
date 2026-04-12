# Getting Started

## Prerequisites

- ESPHome device with UART available
- Roomba with Open Interface (OI) port
- Proper level shifting for UART signals (Roomba side is often 5V)

## 1) Add External Component Source

```yaml
external_components:
  - source: github://retsamedoc/esphome-components@v1.0.0
```

## 2) Configure UART and Component

```yaml
uart:
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 115200

logger:
  baud_rate: 0

roomba:
  id: roomba_ctrl
  use_stream: true
  auto_reconnect: true
  restore_state: false
  brc_pin: GPIO5
```

`brc_pin` defaults to open-drain output; for typical Roomba wiring you only need the pin number as above (the line is pulled up on the robot or your level shifter so it sits high when the pin is released).

## 3) Add Entities

```yaml
sensor:
  - platform: roomba
    roomba_id: roomba_ctrl
    battery:
      name: "Roomba Battery"
    voltage:
      name: "Roomba Voltage"

binary_sensor:
  - platform: roomba
    roomba_id: roomba_ctrl
    bump_left:
      name: "Roomba Bump Left"

text_sensor:
  - platform: roomba
    roomba_id: roomba_ctrl
    charging_state:
      name: "Roomba Charging State"
```

## 4) Flash and Validate

- Build and upload your ESPHome node.
- Confirm entity updates in Home Assistant.
- If no data appears, review `troubleshooting.md`.

## Notes

- The current component exposes sensor, binary sensor, and text sensor platforms.
- ESPHome `vacuum` platform support is not implemented yet.
