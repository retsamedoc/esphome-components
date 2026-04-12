# ESPHome Roomba OI Component

[![CI](https://github.com/retsamedoc/esphome-components/actions/workflows/quality.yaml/badge.svg)](https://github.com/retsamedoc/esphome-components/actions)
[![Issues](https://img.shields.io/github/issues/retsamedoc/esphome-components)](https://github.com/retsamedoc/esphome-components/releases)
[![License](https://img.shields.io/github/license/retsamedoc/esphome-components)](LICENSE)

> **Notice:** The Roomba component is under active development and is not functional yet.

## Overview

The **ESPHome Roomba OI Component** allows ESP32/ESP8266 devices to control iRobot Roomba vacuums
using the Open Interface (OI) via UART.

- Stream mode support with reconnect recovery
- Sensor, binary sensor, and text sensor integration
- Stream/query packets derived from enabled entities
- Packet-table driven decoding
- Optional state restore and hardware wake via BRC pin (open-drain output by default)

[Getting Started ->](getting-started.md)

[Configuration ->](config.md)

[Sensors and Entities ->](sensors.md)

[Architecture (Canonical) ->](architecture.md)

[Troubleshooting ->](troubleshooting.md)

[Compatibility ->](compatibility.md)

[Roadmap ->](roadmap.md)
