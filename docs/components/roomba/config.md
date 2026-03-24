# Configuration

## Example

```yaml
external_components:
  - source: github://retsamedoc/esphome-components@v1.0.0

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
      name: "Battery"

binary_sensor:
  - platform: roomba
    roomba_id: roomba_ctrl
    bump_left:
      name: "Bump Left"
    bump_right:
      name: "Bump Right"

text_sensor:
  - platform: roomba
    roomba_id: roomba_ctrl
    charging_state:
      name: "Charging State"
```


# Options

| Option | Type | Default | Description |
| --- | --- | --- | --- |
| `use_stream` | bool | true | Enable OI stream mode (`STREAM` opcode 148) and the stream frame parser. When `false`, the component polls with `SENSORS` (142) on the same packet list. Use query mode if your robot never sends stream frames (UART works but stream stays silent). |
| `auto_reconnect` | bool | true | Silence-based recovery: after a grace period following each recovery, if no UART bytes arrive for several seconds, the component re-runs the wake/START/SAFE sequence (and stream if enabled). A **minimum interval** between automatic recoveries avoids hammering the robot when stream never starts. Set `false` if you want to avoid reconnects while debugging. |
| `restore_state` | bool | false | Re-issue last command after recovery (non-boot only). |
| `brc_pin` | GPIO | null | Optional wake/reset pin for BRC pulses. |
| `time_id` | time ID | null | Optional ESPHome time source used to send Roomba `SET_DAY_TIME` once when time first becomes valid after boot. |


# Notes

* Disable UART logging to avoid conflicts:

```yaml
logger:
  baud_rate: 0
```
* Beware of IO voltage (Roomba OI is usually 5V)
* Stream mode is preferred for reliability
* Stream/query packet requests are auto-derived from enabled entities in your YAML
* If an entity is not configured, its packet is not requested
* Query-mode fallback uses the same enabled-entity packet list as stream mode
* While docked and charging, some robots may emit an ASCII status line about once per second, for example:
  `bat:   min 28134  sec 33  mV 16643  mA 39  tenths-deg-C 402  mAH 2696  state 11  mode 1`
* Observed field meanings for that ASCII line:
  * `min` + `sec`: appears to be charging elapsed time
  * `mV`: battery voltage (millivolts)
  * `mA`: battery current (milliamps)
  * `tenths-deg-C`: battery temperature in tenths of a degree C (`402` = `40.2 C`)
  * `mAH`: estimated battery charge (mAh)
  * `state` / `mode`: currently undocumented or unknown
* `battery` uses packet `26` and automatically includes packet `25` as a decode dependency
* There is currently no `vacuum:` platform in this component
* To schedule recurring clock syncs, call the `roomba.set_day_time` action from a `time` automation.

## Actions

### `roomba.set_day_time`

Send opcode `168` (`SET_DAY_TIME`) immediately using the configured `time_id` source.

```yaml
- roomba.set_day_time:
    id: roomba_ctrl
```

### `roomba.start_clean`

Send opcode `135` (`CLEAN`).

```yaml
- roomba.start_clean:
    id: roomba_ctrl
```

### `roomba.dock`

Send opcode `143` (`SEEK_DOCK`).

```yaml
- roomba.dock:
    id: roomba_ctrl
```

### `roomba.stop`

Send opcode `173` (`STOP`).

```yaml
- roomba.stop:
    id: roomba_ctrl
```

### `roomba.reset`

Send opcode `7` (`RESET`).

```yaml
- roomba.reset:
    id: roomba_ctrl
```

## Button Example

```yaml
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

