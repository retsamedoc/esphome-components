# Release Notes

## [Unreleased]

- **Breaking:** Component renamed from `roomba_oi` to `roomba` (YAML key, platforms, automation actions). C++ class is now `esphome::roomba::Roomba`. Update `external_components` to `github://retsamedoc/esphome-components@<ref>` when using the new repository. You cannot use `id: roomba` on the `roomba:` block (ESPHome reserves that identifier); use another id such as `roomba_ctrl`.
- Align `current` sensor unit metadata to `mA` to match Roomba packet semantics (no decoder scaling change).

## [1.0.0] - 2026-03-21

- Initial release
- UART Roomba OI driver
- Stream mode support
- Query command path (partial)
- Sensor + binary sensor + text sensor support
- Recovery ladder with hard reset
- Optional state restore
- BRC pin support
