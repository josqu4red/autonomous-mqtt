# autonomous-mqtt - Autonomous desk MQTT remote control

Control your Autonomous desk through MQTT.

Based on [Stefichen5/AutonomousControl](https://github.com/Stefichen5/AutonomousControl) analysis of the command protocol.

## Prerequisites

Using Nix, just run `nix-shell` (or use [nix-direnv](https://github.com/nix-community/nix-direnv)) to get all dependencies set up.

Otherwise follow [ESP-IDF installation procedure](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html).

## Hardware

Part list is basic and sourceable for <$10:
* ESP32 -- it should work with any flavor granted that UART pins are set up accordingly, tested with ESP32-C3 Supermini
* RJ48/50 cable or plug (10 pins) -- usually more expensive than the compute module :upside_down_face:

Connect the RJ50 cable wires to the ESP32 as follow. Pick the GPIOs according to the platform/your choice.
GPIO pins are configurable via `menuconfig`:

| Wire | Signal | OEM remote cable | ESP32 pin |
|------|--------|------------------|-----------|
| 1    | 5V     | Blue (rightmost) | 5V        |
| 2    | TXD    | Green            | GPIO 23   |
| 3    | RXD    | Yellow           | GPIO 22   |
| 4    | GND    | Orange           | GND       |

## Configure the project

Currently all settings must be compiled in: Wifi, MQTT creds, UART pins.

```
idf.py set-target esp32c3   # use your ESP32 flavor
idf.py menuconfig
```

All settings are under **Autonomous controller configuration** (see [Kconfig.projbuild](main/Kconfig.projbuild)):

| Setting | Default | Description |
|---------|---------|-------------|
| Hostname | `autonomous` | mDNS hostname |
| UART port | 2 | UART peripheral number |
| UART RXD pin | 22 | GPIO for RS485 RX |
| UART TXD pin | 23 | GPIO for RS485 TX |
| WiFi SSID | — | Network name |
| WiFi password | — | Network password |
| MQTT broker URL | — | e.g. `mqtt://192.168.1.10` |
| MQTT broker username | — | Broker username |
| MQTT broker password | — | Broker password |
| Position threshold | 2 | Units from target before stopping |
| Idle threshold | 50 | Idle iterations before preset move completes (~1 s) |

## Build and Flash

Build the project and flash it to the board, then run monitor tool to view serial output:

```
idf.py -p PORT flash monitor
```

Replace `PORT` with your serial port, e.g. `/dev/ttyUSB0`.

## MQTT usage

### Position updates

The current desk position is published every second to the state topic:

```
autonomous/<hostname>/state  →  <position byte, hex 0x42–0x83>
```

### Commands

Publish to the command topic to move the desk:

```bash
# Move to absolute height (0x42 = lowest, 0x83 = highest)
mqttui publish autonomous/<hostname>/command height=100

# Activate a memory preset (1–4)
mqttui publish autonomous/<hostname>/command preset=1
```

Position values are in the desk's native unit range (0x42–0x83, height in centimeters). The firmware stops movement once within `POSITION_THRESHOLD` units of the target.

## Protocol

The desk communicates over RS485 at 9600 baud using a proprietary binary protocol.

See [Stefichen5/AutonomousControl](https://github.com/Stefichen5/AutonomousControl) for the full protocol analysis.

## TODO

* Runtime configuration
* Home Assistant MQTT discovery
