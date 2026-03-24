# Troubleshooting

## No UART Data / No Entity Updates

- Ensure `logger.baud_rate: 0` is set to avoid UART conflicts.
- Verify TX/RX wiring and that pins match your YAML.
- Confirm baud rate is `115200`.
- Confirm the Roomba OI port is powered and connected correctly.

## Unstable Data or Frequent Reconnects

- Check grounding between ESP device and Roomba interface.
- Validate level shifting for signal integrity.
- Review power supply stability for both boards.
- If using `brc_pin`, verify pin wiring and polarity.

## Stream Mode Expectations

- Stream mode is the primary runtime path.
- Recovery ladder is silence-based (~3s, ~7s, ~15s thresholds).
- Hard reset step requires `brc_pin` to be configured.

## Query Mode Expectations

- Query fallback polls and decodes one `SENSORS` packet at a time when `use_stream: false`.
- Query packet selection is derived from enabled entities, same as stream mode.
- Stream mode is still preferred for higher update rate.

## Entity-Specific Notes

- If an entity is not configured in YAML, its packet is not requested.
- `battery` percentage uses packet `26` and auto-adds packet `25` dependency.
- `oi_mode` maps to packet `35`; if it is configured, packet `35` is requested and published.
- `current` is published in milliamps (`mA`) to match Roomba packet `23`.

## Roomba Clock Sync (Opcode 168)

- Initial sync only runs if `roomba.time_id` is configured and the time source becomes valid.
- Expected one-time boot log after time becomes valid:
  - `Roomba clock synced from initial valid time`
- Each successful sync also logs the payload values:
  - `Sent SET_DAY_TIME day=<d> hour=<h> minute=<m>`
- If sync does not run, check for warning logs:
  - `Cannot set day/time: no time source configured`
  - `Cannot set day/time: time source not valid yet`
- Daily (or custom cadence) sync is controlled by your `time.on_time` automation calling:
  - `roomba.set_day_time`
- If you want less frequent sync, change the cron schedule in your YAML instead of component code.

## Still Not Working?

- Compare your config against `getting-started.md`.
- Review architecture details in `architecture.md`.
- Check planned fixes in `roadmap.md`.
