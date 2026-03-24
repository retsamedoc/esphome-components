# Roadmap

This file tracks architecture and feature work that is not fully implemented in the current codebase.

## Near Term

## Mid Term

- Expand stream packet list in `start_stream_()`
  - Include packet `35` so `oi_mode` text sensor updates in stream mode
  - Optionally include additional supported packets (43-58 subset)
- Add packet coverage tests
  - Validate packet size table against decode behavior
  - Validate checksum handling and truncated payload behavior
- Improve output registry behavior
  - Allow multiple entity types to observe the same `(packet, index)` key if desired
  - Add diagnostics when keys are overwritten

## Long Term

- Implement an ESPHome `vacuum` platform for `roomba`
  - Wire entity commands to `start_clean()`, `dock()`, and `stop()`
  - Publish vacuum state from packet data and command history
- Auto-select stream packets from configured entities
  - Build packet subscription list from configured sensor/text/binary maps
  - Reduce UART bandwidth and parsing overhead
- Roomba model capability detection
  - Detect supported packet set by model generation
  - Gate packet subscriptions and warnings by capability
- Documentation tooling
  - Generate sensor/packet reference tables from `PACKETS` and Python maps
  - Keep docs synchronized with runtime definitions automatically
- Add an automated unit test harness
  - Introduce Catch2 + CMake + ctest for native parser/decoder tests
  - Cover query fallback and stream regression behavior without hardware

## Completed

- Align current sensor units
  - Updated YAML metadata to `mA` to match Roomba packet semantics
