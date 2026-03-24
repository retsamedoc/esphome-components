# Sensors

This component exposes entities through ESPHome `sensor`, `binary_sensor`, and `text_sensor` platforms.

## Numeric Sensors

| YAML key | Packet | Index | Unit | Notes |
| --- | --- | --- | --- | --- |
| `distance` | 19 | 0 | m | Decoded from signed mm and converted to meters. |
| `angle` | 20 | 0 | deg | Signed angle in degrees. |
| `battery` | 26 | 0 | % | Derived from charge/capacity packets. |
| `voltage` | 22 | 0 | V | Converted from mV in decoder. |
| `current` | 23 | 0 | mA | Decoder publishes raw Roomba current value in milliamps. |
| `temperature` | 24 | 0 | C | Signed temperature value. |

## Binary Sensors

| YAML key | Packet | Index | Notes |
| --- | --- | --- | --- |
| `bump_right` | 7 | 0 | Bump/wheel-drop bitfield decode. |
| `bump_left` | 7 | 1 | Bump/wheel-drop bitfield decode. |
| `wheel_drop_right` | 7 | 2 | Bump/wheel-drop bitfield decode. |
| `wheel_drop_left` | 7 | 3 | Bump/wheel-drop bitfield decode. |
| `wall` | 8 | 0 | Wall detect bit. |
| `cliff_left` | 9 | 0 | Cliff detect bit. |
| `cliff_front_left` | 10 | 0 | Cliff detect bit. |
| `cliff_front_right` | 11 | 0 | Cliff detect bit. |
| `cliff_right` | 12 | 0 | Cliff detect bit. |

## Text Sensors

| YAML key | Packet | Index | Notes |
| --- | --- | --- | --- |
| `charging_state` | 21 | 0 | Enum mapped to readable charging state text. |
| `oi_mode` | 35 | 0 | Enum mapped to OI mode text; requested when configured. |

## Update Behavior

- Stream mode is the primary update path.
- Query mode polls the same enabled-entity packet list as stream mode.
- See `architecture.md` and `roadmap.md` for current limitations.

