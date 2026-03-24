# ESPHome AI Collaboration Guide

This document provides essential context for AI models interacting with this project. Adhering to these guidelines will ensure consistency and maintain code quality.

## 1. Project Overview & Purpose

*   **Primary Goal:** ESPHome is a system to configure microcontrollers (like ESP32, ESP8266, RP2040, and LibreTiny-based chips) using simple yet powerful YAML configuration files. It generates C++ firmware that can be compiled and flashed to these devices, allowing users to control them remotely through home automation systems.
*   **Business Domain:** Internet of Things (IoT), Home Automation.

## 2. Core Technologies & Stack

*   **Languages:** Python (>=3.11), C++ (gnu++20)
*   **Frameworks & Runtimes:** PlatformIO, Arduino, ESP-IDF.
*   **Build Systems:** PlatformIO is the primary build system. CMake is used as an alternative.
*   **Configuration:** YAML.
*   **Key Libraries/Dependencies:**
    *   **Python:** `voluptuous` (for configuration validation), `PyYAML` (for parsing configuration files), `paho-mqtt` (for MQTT communication), `tornado` (for the web server), `aioesphomeapi` (for the native API).
    *   **C++:** `ArduinoJson` (for JSON serialization/deserialization), `AsyncMqttClient-esphome` (for MQTT), `ESPAsyncWebServer` (for the web server).
*   **Package Manager(s):** `pip` (for Python dependencies), `platformio` (for C++/PlatformIO dependencies).
*   **Communication Protocols:** Protobuf (for native API), MQTT, HTTP.

## 3. Architectural Patterns

*   **Overall Architecture:** The project follows a code-generation architecture. The Python code parses user-defined YAML configuration files and generates C++ source code. This C++ code is then compiled and flashed to the target microcontroller using PlatformIO. All of that is handled by the upstream ESPHome project. This project is only for creating an external component.

*   **Directory Structure Philosophy:**
    *   `/components`: Contains the individual components that can be used in ESPHome configurations. Each component is a self-contained unit with its own C++ and Python code. This includes modular hardware and software components with platform-specific implementations and dependency management.

*   **Platform Support:**
    1.  **ESP32** (`components/esp32/`): Espressif ESP32 family. Supports multiple variants (Original, C2, C3, C5, C6, H2, P4, S2, S3) with ESP-IDF framework. Arduino framework supports only a subset of the variants (Original, C3, S2, S3).
    2.  **ESP8266** (`components/esp8266/`): Espressif ESP8266. Arduino framework only, with memory constraints.
    3.  **RP2040** (`components/rp2040/`): Raspberry Pi Pico/RP2040. Arduino framework with PIO (Programmable I/O) support.
    4.  **LibreTiny** (`components/libretiny/`): Realtek and Beken chips. Supports multiple chip families and auto-generated components.

## 4. Coding Conventions & Style Guide

*   **Formatting:**
    *   **Python:** Uses `ruff` and `flake8` for linting and formatting. Configuration is in `pyproject.toml`.
    *   **C++:** Uses `clang-format` for formatting. Configuration is in `.clang-format`.

*   **Naming Conventions:**
    *   **Python:** Follows PEP 8. Use clear, descriptive names following snake_case.
    *   **C++:** Follows the Google C++ Style Guide with these specifics (following clang-tidy conventions):
        - Function, method, and variable names: `lower_snake_case`
        - Class/struct/enum names: `UpperCamelCase`
        - Top-level constants (global/namespace scope): `UPPER_SNAKE_CASE`
        - Function-local constants: `lower_snake_case`
        - Protected/private fields: `lower_snake_case_with_trailing_underscore_`
        - Favor descriptive names over abbreviations

*   **C++ Field Visibility:**
    *   **Prefer `protected`:** Use `protected` for most class fields to enable extensibility and testing. Fields should be `lower_snake_case_with_trailing_underscore_`.
    *   **Use `private` for safety-critical cases:** Use `private` visibility when direct field access could introduce bugs or violate invariants:
        1. **Pointer lifetime issues:** When setters validate and store pointers from known lists to prevent dangling references.
           ```cpp
           // Helper to find matching string in vector and return its pointer
           inline const char *vector_find(const std::vector<const char *> &vec, const char *value) {
             for (const char *item : vec) {
               if (strcmp(item, value) == 0)
                 return item;
             }
             return nullptr;
           }

           class ClimateDevice {
            public:
             void set_custom_fan_modes(std::initializer_list<const char *> modes) {
               this->custom_fan_modes_ = modes;
               this->active_custom_fan_mode_ = nullptr;  // Reset when modes change
             }
             bool set_custom_fan_mode(const char *mode) {
               // Find mode in supported list and store that pointer (not the input pointer)
               const char *validated_mode = vector_find(this->custom_fan_modes_, mode);
               if (validated_mode != nullptr) {
                 this->active_custom_fan_mode_ = validated_mode;
                 return true;
               }
               return false;
             }
            private:
             std::vector<const char *> custom_fan_modes_;  // Pointers to string literals in flash
             const char *active_custom_fan_mode_{nullptr};  // Must point to entry in custom_fan_modes_
           };
           ```
        2. **Invariant coupling:** When multiple fields must remain synchronized to prevent buffer overflows or data corruption.
           ```cpp
           class Buffer {
            public:
             void resize(size_t new_size) {
               auto new_data = std::make_unique<uint8_t[]>(new_size);
               if (this->data_) {
                 std::memcpy(new_data.get(), this->data_.get(), std::min(this->size_, new_size));
               }
               this->data_ = std::move(new_data);
               this->size_ = new_size;  // Must stay in sync with data_
             }
            private:
             std::unique_ptr<uint8_t[]> data_;
             size_t size_{0};  // Must match allocated size of data_
           };
           ```
        3. **Resource management:** When setters perform cleanup or registration operations that derived classes might skip.
    *   **Provide `protected` accessor methods:** When derived classes need controlled access to `private` members.

*   **C++ Preprocessor Directives:**
    *   **Avoid `#define` for constants:** Using `#define` for constants is discouraged and should be replaced with `const` variables or enums.
    *   **Use `#define` only for:**
        - Conditional compilation (`#ifdef`, `#ifndef`)
        - Compile-time sizes calculated during Python code generation (e.g., configuring `std::array` or `StaticVector` dimensions via `cg.add_define()`)

*   **C++ Additional Conventions:**
    *   **Member access:** Prefix all class member access with `this->` (e.g., `this->value_` not `value_`)
    *   **Indentation:** Use spaces (two per indentation level), not tabs
    *   **Type aliases:** Prefer `using type_t = int;` over `typedef int type_t;`
    *   **Line length:** Wrap lines at no more than 120 characters

*   **Component Structure:**
    *   **Standard Files:**
        ```
        components/[component_name]/
        ├── __init__.py          # Component configuration schema and code generation
        ├── [component].h        # C++ header file (if needed)
        ├── [component].cpp      # C++ implementation (if needed)
        └── [platform]/          # Platform-specific implementations
            ├── __init__.py      # Platform-specific configuration
            ├── [platform].h     # Platform C++ header
            └── [platform].cpp   # Platform C++ implementation
        ```

    *   **Component Metadata:**
        - `DEPENDENCIES`: List of required components
        - `AUTO_LOAD`: Components to automatically load
        - `CONFLICTS_WITH`: Incompatible components
        - `CODEOWNERS`: GitHub usernames responsible for maintenance
        - `MULTI_CONF`: Whether multiple instances are allowed

*   **Code Generation & Common Patterns:**
    *   **Configuration Schema Pattern:**
        ```python
        import esphome.codegen as cg
        import esphome.config_validation as cv
        from esphome.const import CONF_KEY, CONF_ID

        CONF_PARAM = "param"  # A constant that does not yet exist in esphome/const.py

        my_component_ns = cg.esphome_ns.namespace("my_component")
        MyComponent = my_component_ns.class_("MyComponent", cg.Component)

        CONFIG_SCHEMA = cv.Schema({
            cv.GenerateID(): cv.declare_id(MyComponent),
            cv.Required(CONF_KEY): cv.string,
            cv.Optional(CONF_PARAM, default=42): cv.int_,
        }).extend(cv.COMPONENT_SCHEMA)

        async def to_code(config):
            var = cg.new_Pvariable(config[CONF_ID])
            await cg.register_component(var, config)
            cg.add(var.set_key(config[CONF_KEY]))
            cg.add(var.set_param(config[CONF_PARAM]))
        ```

    *   **C++ Class Pattern:**
        ```cpp
        namespace esphome::my_component {

        class MyComponent : public Component {
         public:
          void setup() override;
          void loop() override;
          void dump_config() override;

          void set_key(const std::string &key) { this->key_ = key; }
          void set_param(int param) { this->param_ = param; }

         protected:
          std::string key_;
          int param_{0};
        };

        }  // namespace esphome::my_component
        ```

    *   **Common Component Examples:**
        - **Sensor:**
          ```python
          from esphome.components import sensor
          CONFIG_SCHEMA = sensor.sensor_schema(MySensor).extend(cv.polling_component_schema("60s"))
          async def to_code(config):
              var = await sensor.new_sensor(config)
              await cg.register_component(var, config)
          ```

        - **Binary Sensor:**
          ```python
          from esphome.components import binary_sensor
          CONFIG_SCHEMA = binary_sensor.binary_sensor_schema().extend({ ... })
          async def to_code(config):
              var = await binary_sensor.new_binary_sensor(config)
          ```

        - **Switch:**
          ```python
          from esphome.components import switch
          CONFIG_SCHEMA = switch.switch_schema().extend({ ... })
          async def to_code(config):
              var = await switch.new_switch(config)
          ```

*   **Configuration Validation:**
    *   **Common Validators:** `cv.int_`, `cv.float_`, `cv.string`, `cv.boolean`, `cv.int_range(min=0, max=100)`, `cv.positive_int`, `cv.percentage`.
    *   **Complex Validation:** `cv.All(cv.string, cv.Length(min=1, max=50))`, `cv.Any(cv.int_, cv.string)`.
    *   **Platform-Specific:** `cv.only_on(["esp32", "esp8266"])`, `esp32.only_on_variant(...)`, `cv.only_on_esp32`, `cv.only_on_esp8266`, `cv.only_on_rp2040`.
    *   **Framework-Specific:** `cv.only_with_framework(...)`, `cv.only_with_arduino`, `cv.only_with_esp_idf`.
    *   **Schema Extensions:**
        ```python
        CONFIG_SCHEMA = cv.Schema({ ... })
         .extend(cv.COMPONENT_SCHEMA)
         .extend(uart.UART_DEVICE_SCHEMA)
         .extend(i2c.i2c_device_schema(0x48))
         .extend(spi.spi_device_schema(cs_pin_required=True))
        ```

## 5. Specific Instructions for AI Collaboration

*   **Documentation Contributions:**
    *   Documentation is hosted in the 'docs` folder.
        *   When editing a component's documentation page, also update the corresponding component index page to ensure both pages remain in sync.

*   **Best Practices:**
    *   **Component Development:** Keep dependencies minimal, provide clear error messages, and write comprehensive docstrings and tests.
    *   **Code Generation:** Generate minimal and efficient C++ code. Validate all user inputs thoroughly. Support multiple platform variations.
    *   **Configuration Design:** Aim for simplicity with sensible defaults, while allowing for advanced customization.
    *   **Embedded Systems Optimization:** ESPHome targets resource-constrained microcontrollers. Be mindful of flash size and RAM usage.

        **Why Heap Allocation Matters:**

        ESP devices run for months with small heaps shared between Wi-Fi, BLE, LWIP, and application code. Over time, repeated allocations of different sizes fragment the heap. Failures happen when the largest contiguous block shrinks, even if total free heap is still large. We have seen field crashes caused by this.

        **Heap allocation after `setup()` should be avoided unless absolutely unavoidable.** Every allocation/deallocation cycle contributes to fragmentation. ESPHome treats runtime heap allocation as a long-term reliability bug, not a performance issue. Helpers that hide allocation (`std::string`, `std::to_string`, string-returning helpers) are being deprecated and replaced with buffer and view based APIs.

        **STL Container Guidelines:**

        ESPHome runs on embedded systems with limited resources. Choose containers carefully:

        1. **Compile-time-known sizes:** Use `std::array` instead of `std::vector` when size is known at compile time.
           ```cpp
           // Bad - generates STL realloc code
           std::vector<int> values;

           // Good - no dynamic allocation
           std::array<int, MAX_VALUES> values;
           ```
           Use `cg.add_define("MAX_VALUES", count)` to set the size from Python configuration.

           **For byte buffers:** Avoid `std::vector<uint8_t>` unless the buffer needs to grow. Use `std::unique_ptr<uint8_t[]>` instead.

           > **Note:** `std::unique_ptr<uint8_t[]>` does **not** provide bounds checking or iterator support like `std::vector<uint8_t>`. Use it only when you do not need these features and want minimal overhead.

           ```cpp
           // Bad - STL overhead for simple byte buffer
           std::vector<uint8_t> buffer;
           buffer.resize(256);

           // Good - minimal overhead, single allocation
           std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(256);
           // Or if size is constant:
           std::array<uint8_t, 256> buffer;
           ```

        2. **Compile-time-known fixed sizes with vector-like API:** Use `StaticVector` from `esphome/core/helpers.h` for compile-time fixed size with `push_back()` interface (no dynamic allocation).
           ```cpp
           // Bad - generates STL realloc code (_M_realloc_insert)
           std::vector<ServiceRecord> services;
           services.reserve(5);  // Still includes reallocation machinery

           // Good - compile-time fixed size, no dynamic allocation
           StaticVector<ServiceRecord, MAX_SERVICES> services;
           services.push_back(record1);
           ```
           Use `cg.add_define("MAX_SERVICES", count)` to set the size from Python configuration.
           Like `std::array` but with vector-like API (`push_back()`, `size()`) and no STL reallocation code.

        3. **Runtime-known sizes:** Use `FixedVector` from `esphome/core/helpers.h` when the size is only known at runtime initialization.
           ```cpp
           // Bad - generates STL realloc code (_M_realloc_insert)
           std::vector<TxtRecord> txt_records;
           txt_records.reserve(5);  // Still includes reallocation machinery

           // Good - runtime size, single allocation, no reallocation machinery
           FixedVector<TxtRecord> txt_records;
           txt_records.init(record_count);  // Initialize with exact size at runtime
           ```
           **Benefits:**
           - Eliminates `_M_realloc_insert`, `_M_default_append` template instantiations (saves 200-500 bytes per instance)
           - Single allocation, no upper bound needed
           - No reallocation overhead
           - Compatible with protobuf code generation when using `[(fixed_vector) = true]` option

        4. **Small datasets (1-16 elements):** Use `std::vector` or `std::array` with simple structs instead of `std::map`/`std::set`/`std::unordered_map`.
           ```cpp
           // Bad - 2KB+ overhead for red-black tree/hash table
           std::map<std::string, int> small_lookup;
           std::unordered_map<int, std::string> tiny_map;

           // Good - simple struct with linear search (std::vector is fine)
           struct LookupEntry {
             const char *key;
             int value;
           };
           std::vector<LookupEntry> small_lookup = {
             {"key1", 10},
             {"key2", 20},
             {"key3", 30},
           };
           // Or std::array if size is compile-time constant:
           // std::array<LookupEntry, 3> small_lookup = {{ ... }};
           ```
           Linear search on small datasets (1-16 elements) is often faster than hashing/tree overhead, but this depends on lookup frequency and access patterns. For frequent lookups in hot code paths, the O(1) vs O(n) complexity difference may still matter even for small datasets. `std::vector` with simple structs is usually fine—it's the heavy containers (`map`, `set`, `unordered_map`) that should be avoided for small datasets unless profiling shows otherwise.

        5. **Avoid `std::deque`:** It allocates in 512-byte blocks regardless of element size, guaranteeing at least 512 bytes of RAM usage immediately. This is a major source of crashes on memory-constrained devices.

        6. **Detection:** Look for these patterns in compiler output:
           - Large code sections with STL symbols (vector, map, set)
           - `alloc`, `realloc`, `dealloc` in symbol names
           - `_M_realloc_insert`, `_M_default_append` (vector reallocation)
           - Red-black tree code (`rb_tree`, `_Rb_tree`)
           - Hash table infrastructure (`unordered_map`, `hash`)

        **Prioritize optimization effort for:**
        - Core components (API, network, logger)
        - Widely-used components (mdns, wifi, ble)
        - Components causing flash size complaints

        Note: Avoiding heap allocation after `setup()` is always required regardless of component type. The prioritization above is about the effort spent on container optimization (e.g., migrating from `std::vector` to `StaticVector`).

    *   **State Management:** Use `CORE.data` for component state that needs to persist during configuration generation. Avoid module-level mutable globals.

        **Bad Pattern (Module-Level Globals):**
        ```python
        # Don't do this - state persists between compilation runs
        _component_state = []
        _use_feature = None

        def enable_feature():
            global _use_feature
            _use_feature = True
        ```

        **Bad Pattern (Flat Keys):**
        ```python
        # Don't do this - keys should be namespaced under component domain
        MY_FEATURE_KEY = "my_component_feature"
        CORE.data[MY_FEATURE_KEY] = True
        ```

        **Good Pattern (dataclass):**
        ```python
        from dataclasses import dataclass, field
        from esphome.core import CORE

        DOMAIN = "my_component"

        @dataclass
        class MyComponentData:
            feature_enabled: bool = False
            item_count: int = 0
            items: list[str] = field(default_factory=list)

        def _get_data() -> MyComponentData:
            if DOMAIN not in CORE.data:
                CORE.data[DOMAIN] = MyComponentData()
            return CORE.data[DOMAIN]

        def request_feature() -> None:
            _get_data().feature_enabled = True

        def add_item(item: str) -> None:
            _get_data().items.append(item)
        ```

        If you need a real-world example, search for components that use `@dataclass` with `CORE.data` in the codebase. Note: Some components may use `TypedDict` for dictionary-based storage; both patterns are acceptable depending on your needs.

        **Why this matters:**
        - Module-level globals persist between compilation runs if the dashboard doesn't fork/exec
        - `CORE.data` automatically clears between runs
        - Namespacing under `DOMAIN` prevents key collisions between components
        - `@dataclass` provides type safety and cleaner attribute access

*   **Security:** Be mindful of security when making changes to the API, web server, or any other network-related code. Do not hardcode secrets or keys.

*   **Dependencies & Build System Integration:**
    *   **Python:** When adding a new Python dependency, add it to the appropriate `requirements*.txt` file and `pyproject.toml`.
    *   **C++ / PlatformIO:** When adding a new C++ dependency, add it to `platformio.ini` and use `cg.add_library`.
    *   **Build Flags:** Use `cg.add_build_flag(...)` to add compiler flags.

## Local lint and format (this repository)

This external component repo keeps tooling in the root: [pyproject.toml](../pyproject.toml) (Ruff + Pylint), [.clang-format](../.clang-format), [.clang-tidy](../.clang-tidy). **Python** sources under `components/` use `ruff check` and `ruff format` (120-column line length, see `[tool.ruff]`). **C++** uses `clang-format` only; do not point Ruff at `.h` / `.cpp`. CI runs `ruff check`, `ruff format --check`, and `clang-format --dry-run` on `components/roomba_oi`. Optional local `clang-tidy` needs a `compile_commands.json` from a firmware build.
