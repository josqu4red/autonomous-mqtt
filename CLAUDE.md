# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

ESP32 firmware that bridges an Autonomous electric standing desk to MQTT. The ESP32 connects to the desk via UART (RS485 half-duplex) and exposes control via WiFi/MQTT.

## Development Environment

This project uses Nix for dependency management. Activate the dev shell before any ESP-IDF commands:

```bash
nix-shell   # or use nix-direnv for automatic activation
```

## Common Commands

```bash
idf.py menuconfig              # Configure WiFi, MQTT, UART pins, credentials
idf.py build                   # Build the firmware
idf.py -p PORT flash monitor   # Flash and open serial monitor
idf.py monitor                 # Serial monitor only (after flashing)
```

## Architecture

### Communication Stack

```
MQTT Broker ←→ WiFi ←→ ESP32 ←→ UART RS485 ←→ Autonomous Desk
```

### Source Files (`main/`)

| File | Responsibility |
|------|---------------|
| `main.c` | Entry point; owns `_Atomic position_t`, `atomic_bool cancel`, `desk_ctx_t`; spawns all tasks |
| `desk.c` / `desk.h` | Desk movement logic, preset commands, UART command encoding; defines `presets`, `state_topic`, `command_topic` |
| `uart.c` / `uart.h` | UART event loop, RS485 half-duplex, frame accumulator, position decoding |
| `mqtt.c` / `mqtt.h` | MQTT client init and error logging |
| `wifi.c` / `wifi.h` | WiFi station mode, indefinite reconnect loop |
| `Kconfig.projbuild` | All configurable settings (menuconfig definitions) |

### MQTT Protocol

- **Subscribe** `autonomous/desk1/set`: accepts `height=<66-131>` or `preset=<1-4>`
- **Publish** `autonomous/desk1/data`: current height as integer, every 1 second

### Desk UART Protocol

Commands sent to desk: `0xD8 0xD8 0x66 <button_code>`

Button codes: `0x00` (start), `0x01` (down), `0x02` (up), `0x04`/`0x08`/`0x10`/`0x20` (presets 1–4), `0x40` (M)

Position responses from desk: `0x98 0x98 [0x00|0x03] [0x00|0x03] <pos> <pos>`
Position range: `0x42` (66cm low) to `0x83` (131cm high)

UART reads use a frame accumulator (`FRAME_BUF = 11` bytes) to handle partial packets — bytes are buffered until a complete 6-byte frame starting with `0x98 0x98` is found.

### FreeRTOS Tasks

1. **UART Event Handler** (priority 2): drains UART into frame accumulator, decodes position, atomically updates shared state
2. **Desk Control Task** (priority 3): blocks on a depth-1 command queue; executes `go_to_height`/`go_to_preset`
3. **MQTT Publish Task** (priority 5): publishes position every 1 second
4. **Main loop**: handles WiFi and MQTT events (non-blocking — WiFi connects asynchronously)

### Shared State (`main.c`)

```c
_Atomic position_t position;   // written by UART task, read by all others
atomic_bool        cancel;     // set by MQTT handler to interrupt in-progress movement
desk_ctx_t         ctx;        // bundles position, cmd_queue, cancel — passed to MQTT handler and desk task
```

All cross-task position reads use `atomic_load`, writes use `atomic_store` (C11 atomics, no mutex).

### Movement Logic

- `go_to_height`: sends up/down commands until `|current - desired| <= position_threshold (2)`; checks `cancel` each iteration
- `go_to_preset`: sends preset button until position is idle for `idle_threshold (50)` consecutive readings; checks `cancel` each iteration
- A new MQTT command sets `cancel=true` and calls `xQueueOverwrite` — the in-progress move aborts within one `SEND_DELAY (100ms)` tick

### WiFi Reconnection

`wifi_init` returns immediately after `esp_wifi_start`. On `WIFI_EVENT_STA_DISCONNECTED` the handler waits 1 s and calls `esp_wifi_connect` unconditionally — no retry limit.

## Configuration

All runtime settings are defined in `Kconfig.projbuild` and set via `idf.py menuconfig`:

- `AUTONOMOUS_HOSTNAME`: mDNS/network hostname (default: `autonomous`)
- UART port number and GPIO pins (RXD/TXD, default: GPIO 22/23)
- WiFi SSID, password, auth mode
- MQTT broker URL, username, password

Defaults are in `sdkconfig.defaults` (sets log level to INFO/DEBUG).
