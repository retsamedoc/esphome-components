# Compatibility

## Current Support Status

| Area | Status | Notes |
| --- | --- | --- |
| ESPHome external component | Supported | Installed via `external_components`. |
| UART transport | Supported | 115200 baud expected. |
| Sensor platform (`sensor`) | Supported | See `sensors.md` for full map. |
| Binary sensor platform (`binary_sensor`) | Supported | See `sensors.md` for full map. |
| Text sensor platform (`text_sensor`) | Supported | Includes charging state and OI mode mappings. |
| Vacuum platform (`vacuum`) | Not implemented | Tracked in `roadmap.md`. |

## Roomba Series Notes

The protocol table includes packet definitions for multiple Roomba generations, but practical compatibility depends on model capability and wiring.

| Roomba series family | Expected compatibility |
| --- | --- |
| 500 series | Basic packet set expected to work. |
| 600 series | Basic packet set expected to work. |
| 700/900 series | Packet definitions exist; validate model-specific behavior. |

## Electrical and Interface Notes

- Roomba OI interfaces are commonly 5V TTL.
- Use suitable level shifting when MCU UART is 3.3V.
- Verify shared ground between ESP device and Roomba interface.

## Runtime Caveats

- Stream mode is the primary supported data path.
- Query mode behavior is partial.
- See `architecture.md` and `troubleshooting.md` for details.
