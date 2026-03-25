# ESPHome External Components Collection

![CI](https://github.com/retsamedoc/esphome-components/actions/workflows/quality.yaml/badge.svg)
![License](https://img.shields.io/github/license/retsamedoc/esphome-components)
![Maintenance](https://img.shields.io/badge/Maintained-yes-green)
![Issues](https://img.shields.io/github/issues/retsamedoc/esphome-components)

## ![ESPHome logo](esphome-logo.png)  What is ESPHome?

[ESPHome](https://esphome.io/) is an open-source platform for programming ESP8266 and ESP32 devices using simple YAML configuration. It integrates seamlessly with Home Assistant and enables powerful, local-first smart home automation.


## 🔌 About This Repository

This repository provides a collection of **external components** that extend ESPHome beyond its core capabilities.

### Goals

* 🧩 Support devices not readily available upstream
* 🧪 Enable advanced or experimental features
* ⚙️ Provide deeper hardware and protocol integrations

All components are designed to be easily included using ESPHome’s `[external_component](https://esphome.io/components/external_components)` feature.


### 🛠 Installation

Copy/paste the following into your project's YAML, then follow the individual modules' instructions.

```yaml
external_components:
  - source: github://retsamedoc/esphome-components
```

## Available Components

### `roomba`

Control iRobot&reg; devices that follow the [`Roomba® Open Interface (OI) Specification`](/docs/roomba/references/iRobot%20Roomba%20600%20Open%20Interface%20Spec.pdf) using ESPHome and the `[UART](https://esphome.io/components/uart)` component.

> **Notice:** This component is under active development and is not functional yet.

Key features:

- Stream mode with reconnect recovery (and a fallback to query mode)
- Battery Health (voltage, current), Cleaning State, and telemetry (distance, angle) sensors
- Command automation actions for CLEAN/SEEK_DOCK/STOP/RESET
- Hardware wake/reset via BRC pin (optional but recommended!)
- Optional Roomba time/date sync via the `[time](https://esphome.io/components/time)` component.

More information (including detailed instructions) can be found in the component's docs: [`docs/roomba/index.md`](https://retsamedoc.github.io/esphome-components/roomba/).


## 🤝 Contributing

Contributions encouraged!

- 🐛 Report bugs via Issues  
- 💡 Propose features or new device support  
- 🔧 Submit pull requests for fixes or enhancements  

If you're unsure about an idea, feel free to open an issue to discuss it first.

## 📄 License

MIT

---

<p align="center">
  Built for the ESPHome community ❤️
</p>
