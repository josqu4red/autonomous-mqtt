# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

ESP32 firmware that bridges an Autonomous brand standing desk's proprietary RS485/UART serial protocol to MQTT, enabling home automation control. Written in C using ESP-IDF and FreeRTOS.

## Development Environment

This project uses Nix for dependency management. Enter the dev shell before running any ESP-IDF commands:

```bash
nix-shell   # or nix-direnv if configured
```

Without Nix: follow the [ESP-IDF installation procedure](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html) manually.

## Common Commands

```bash
# Configure WiFi credentials, MQTT broker URL/credentials, UART pins, hostname
idf.py menuconfig

# Build the firmware
idf.py build

# Flash to ESP32 and open serial monitor (replace PORT with e.g. /dev/ttyUSB0)
idf.py -p PORT flash monitor
```

There is no automated test suite — validation is done via serial monitor output and MQTT message publishing.

## Configuration

All configurable parameters are under `Kconfig.projbuild` and set via `menuconfig` → "Autonomous Controller":
- Hostname (default: `autonomous`)
- UART port, RXD/TXD pin numbers
- WiFi SSID, password, auth mode
- MQTT broker URL, username, password

## Architecture

**3 FreeRTOS tasks** run concurrently, sharing a single `position_t` variable passed by reference (no mutex — relies on atomic 8-bit reads on ESP32):

| Task | Source | Role |
|------|--------|------|
| `uart_event_handler` | `uart.c` | Reads 6-byte position frames from desk over RS485, decodes position |
| `mqtt_event_handler` | `main.c` | Receives commands on `autonomous/desk1/set`, drives desk movement |
| `mqtt_publish_position` | `main.c` | Publishes current position to `autonomous/desk1/data` every 1 second |

**MQTT command format** (topic: `autonomous/desk1/set`):
- `height=XXX` — move to absolute position (range 0x42–0x83)
- `preset=Y` — activate desk preset (1–4)

**Desk serial protocol:**
- Commands to desk: 5 bytes — `0xD8 0xD8 0x66 <button_byte> <button_byte>`
- Responses from desk: 6 bytes — `0x98 0x98 0x00/0x03 0x00/0x03 <pos_high> <pos_low>`

**Module responsibilities:**
- `main.c` — entry point, subsystem init, MQTT event dispatch and movement logic
- `desk.c/h` — `go_to_height()` and `go_to_preset()` movement algorithms with position thresholds
- `uart.c/h` — RS485 UART driver, protocol encode/decode, position update loop
- `mqtt.c/h` — MQTT client init and broker connection wrapper
- `wifi.c/h` — WiFi station mode setup and reconnection handling
