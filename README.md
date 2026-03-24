# ESPHome External Components Collection


![CI](https://github.com/retsamedoc/esphome-components/actions/workflows/quality.yaml/badge.svg)
![GitHub release](https://img.shields.io/github/v/release/retsamedoc/esphome-components)
[![Docs](https://img.shields.io/badge/docs-online-brightgreen)](https://retsamedoc.github.io/esphome-components/)
![License](https://img.shields.io/github/license/retsamedoc/esphome-components)
![ESPHome](https://img.shields.io/badge/ESPHome-compatible-blue)
![Maintenance](https://img.shields.io/badge/Maintained-yes-green)
![Issues](https://img.shields.io/github/issues/retsamedoc/esphome-components)

Reusable external components for ESPHome, maintained in a single repository.

## Documentation

[Project docs (MkDocs)](https://retsamedoc.github.io/esphome-components/)

## Available Components

### `roomba`

Control iRobot Roomba robots via the Open Interface (OI) using ESPHome over UART.

> **Notice:** This component is under active development and is not functional yet.

Key features:

- Full UART-based OI driver
- Stream mode with reconnect recovery (grace period and minimum interval between automatic recoveries; use query mode if stream never produces data)
- Sensor, binary sensor, and text sensor bindings
- Stream/query packets auto-selected from enabled YAML entities
- Battery, voltage, current, distance, angle, and diagnostics sensors
- Auto-reconnect with watchdog
- Optional restore of cleaning state
- Hardware wake/reset via BRC pin
- Optional clock sync via `time_id` and `roomba.set_day_time`
- Command automation actions for CLEAN/SEEK_DOCK/STOP/RESET

Component docs: [`docs/components/roomba/index.md`](docs/components/roomba/index.md)

## Installation (Roomba Example)

```yaml
external_components:
  - source: github://retsamedoc/esphome-components@v1.0.0
```

## Example (Roomba)

Use a distinct `id` for the `roomba:` block (the examples use `roomba_ctrl`); ESPHome does not allow `id: roomba` because it matches the integration key.

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
  time_id: sntp_time

time:
  - platform: sntp
    id: sntp_time
    on_time:
      - seconds: 0
        minutes: 0
        hours: 3
        then:
          - roomba.set_day_time:
              id: roomba_ctrl

sensor:
  - platform: roomba
    roomba_id: roomba_ctrl
    battery:
      name: "Roomba Battery"

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

button:
  - platform: template
    name: "Roomba Clean"
    on_press:
      - roomba.start_clean:
          id: roomba_ctrl

  - platform: template
    name: "Roomba Dock"
    on_press:
      - roomba.dock:
          id: roomba_ctrl

  - platform: template
    name: "Roomba Stop"
    on_press:
      - roomba.stop:
          id: roomba_ctrl

  - platform: template
    name: "Roomba Reset"
    on_press:
      - roomba.reset:
          id: roomba_ctrl
```


## Hardware Notes (Roomba)

* Roomba OI port (Mini-DIN)
* Use level shifting (Roomba is often 5V)
* BRC pin optional but recommended (long reset pulse once at boot when configured, then short wake on each recovery)
* Disable UART logger (`logger.baud_rate: 0`) to avoid serial contention
* If UART debug shows ASCII lines (for example `bat: ... state 11 ... mode 1`), that is not binary OI stream data; use `use_stream: false` and rely on query mode. Stream frames must start with byte `0x13` (`19`).
* While docked and charging, that ASCII `bat:` line may appear about once per second. Observed fields are: `min`/`sec` (appears to be elapsed charge time), `mV`, `mA`, `tenths-deg-C` (`402` = `40.2 C`), and `mAH` (estimated charge). `state` and `mode` are currently undocumented/unknown.


## Development

Lint and format **Python** with [Ruff](https://docs.astral.sh/ruff/) using [pyproject.toml](pyproject.toml). Format **C++** with [clang-format](https://clang.llvm.org/docs/ClangFormat.html) using [.clang-format](.clang-format) in the repo root. These are separate tools: **do not** use Ruff on `.h` / `.cpp` files.

From the repository root (Python 3.11+):

```bash
pip install ruff
ruff check components/
ruff format components/
```

Check formatting without writing files: `ruff format --check components/`.

Optional [Pylint](https://pylint.readthedocs.io/) (configuration under `[tool.pylint]` in `pyproject.toml`):

```bash
pip install pylint
pylint components/
```

C++ (component sources only):

```bash
find components/roomba -type f \( -name '*.h' -o -name '*.cpp' \) -print0 | xargs -0 clang-format --dry-run --Werror
```

To apply C++ formatting in place, replace `--dry-run --Werror` with `-i`.

[clang-tidy](https://clang.llvm.org/extra/clang-tidy/) (optional): requires a `compile_commands.json` produced by an ESPHome / PlatformIO build that includes this component. Run with `-p <build_dir>` and scope to `components/roomba/` first. This repo’s [.clang-tidy](.clang-tidy) sets `WarningsAsErrors: '*'`, so expect to iterate on checks or NOLINTs when including ESPHome headers.

More detail for AI and contributors: [.ai/INSTRUCTIONS.md](.ai/INSTRUCTIONS.md).

## Status

The `roomba` component is under active development and is not functional yet. Contributions welcome.

See the docs site for architecture, compatibility, troubleshooting, and roadmap details for each component.


## License

MIT

