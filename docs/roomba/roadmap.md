# Roadmap

This file tracks architecture and feature work that is not fully implemented in the current codebase.

## Near Term
- **Activity state text sensor:** derive and publish high-level state as one text sensor: **Docked**, **Cleaning**, and **Idle** (not docked and not cleaning), from existing packet data (for example current, charging state, and related signals). This becomes the basis for smarter BRC and dock-targeted behavior later.
- Improve startup and recovery sequences to improve OI mode initialization.
- Convert `send_opcode()` from a rate limiting function to a message queue
  - Instead of dropping packets, we queue them up to ensure nothing is missed
- Convert protocol hooks (Opcodes + `PACKET[]`) into generated manifest
  - Build YAML-based manifest to describe entire OI protocol
  - Create codegen script to provide both python and C++ from manifest
  - Refactor code to switch `PACKETS[]` -> codegen `ENTITIES[]`

## Mid Term
- **BRC watchdog (periodic keep-awake):** optional timed BRC nudges when the robot might sleep without UART traffic, analogous to upstream “lazy 650+” periodic wakeup (interval and pulse policy TBD; see `docs/roomba/references/RoombaComponent-wakeup-spec.md` for reference behavior).
- **Dock wake routine:** when activity state indicates docked, run a BRC pulse followed by the documented Clean/Dock opcode sequence so docked units stay responsive—**depends on the activity state text sensor** above. Implement as a dedicated helper aligned with existing names (`nudge_roomba_()`, `wake_roomba_()`, etc.), not as a port of upstream symbol names.
- Capture and Interprete non-OI mode Strings
  - Use as alternate sources while sleeping on dock
  - Capture additional diagnostic info at Roomba boot (firmware version, etc)
- Roomba OI Version Gate
  - Configuration item to specify Roomba model
  - Gate packet subscriptions and warnings by capability
- Expand stream packet list in `start_stream_()`
  - Optionally include additional supported packets (43-58 subset)
- Add packet coverage tests
  - Validate packet size table against decode behavior
  - Validate checksum handling and truncated payload behavior
- Improve output registry behavior
  - Allow multiple entity types to observe the same `(packet, index)` key if desired
  - Add diagnostics when keys are overwritten
  - Might ride along for free w/ protocol manifest...

## Long Term
- Create better hardware documentation
  - Add wiring diagram for ESP32 devices to roomba
  - Include optional features (Beacon) and hardware selection matrix
- Add missing `Locate` command using `song` and `play` commands
  - Create method to preload/override stock sounds as configuration option.
- Auto-select stream packets from configured entities
  - Build packet subscription list from configured sensor/text/binary maps
  - Reduce UART bandwidth and parsing overhead
- Roomba OI Version Gate
  - Add auto-detect model->OI version if not specified
  - Warn if specified model mismaches with auto-detect algorithm
- Add an automated unit test harness
  - Introduce Catch2 + CMake + ctest for native parser/decoder tests
  - Cover query fallback and stream regression behavior without hardware

