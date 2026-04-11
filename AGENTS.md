# AGENTS.md

## Cursor Cloud specific instructions

This is an **ESPHome external components** repository (not a standalone application). The primary development loop is: edit Python/C++ component code under `components/`, then validate with lint and ESPHome config/compile commands.

### Quick reference

| Task | Command |
|---|---|
| Python lint | `ruff check components/roomba` |
| Python format check | `ruff format --check components/roomba` |
| C++ format check | `find components/roomba -type f \( -name '*.h' -o -name '*.cpp' \) -print0 \| xargs -0 -r clang-format --dry-run --Werror` |
| Validate config | `esphome config examples/roomba.yaml` |
| Generate C++ (no toolchain) | `esphome compile examples/roomba.yaml --only-generate` |
| Full firmware compile | `esphome compile examples/roomba.yaml` (downloads ESP-IDF toolchain, ~10 min first run) |

### Notes

- `~/.local/bin` must be on `PATH` for `esphome` and `ruff` CLI commands (added to `~/.bashrc` during setup).
- There are no unit tests in this repository. CI quality checks are: `ruff check`, `ruff format --check`, and `clang-format --dry-run --Werror` on the `components/` directory.
- Build validation in CI uses `esphome config` + `esphome compile --only-generate` against `examples/roomba.yaml`.
- Full `esphome compile` (without `--only-generate`) downloads the PlatformIO ESP-IDF toolchain on first run, which is large. Use `--only-generate` for quick feedback during development.
- Coding conventions and component architecture patterns are documented in `.ai/INSTRUCTIONS.md`.
- Ruff and clang-format configurations live in `pyproject.toml` and `.clang-format` respectively.
