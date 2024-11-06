# autonomous-mqtt - Autonomous desk MQTT remote control

Control your Autonomous desk through MQTT.

Based on [Stefichen5/AutonomousControl](https://github.com/Stefichen5/AutonomousControl) analysis of the command protocol.

## Building

### Prerequisites

Using [Nix](https://nixos.org/), just run `nix-shell` (or use [nix-direnv](https://github.com/nix-community/nix-direnv)) to get all dependencies set up.

Otherwise follow [ESP-IDF installation procedure](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html).

### Configure the project

Currently all settings must be compiled in: Wifi, MQTT creds, UART pins.

```
idf.py set-target esp32c3   # use your ESP32 flavor
idf.py menuconfig
```

There is an Autonomous section with relevant settings (see [project config](main/Kconfig.projbuild)).

### Build and Flash

Build the project and flash it to the board, then run monitor tool to view serial output:

```
idf.py -p PORT flash monitor
```

## Hardware

### Parts

Part list is pretty basic and sourceable for <$10:
* ESP32 -- it should work with any flavor granted that UART pins are set up accordingly, tested with ESP32-C3 Supermini
* RJ48/50 cable or plug (10 pins) -- usually more expensive than the compute module :upside_down_face:

### Assembly


## Interface

### Command

The module subscribes to a command topic: `autonomous/<hostname>/set`

It expects either a `height` in centimeters or a `preset` from 1 to 4 (remote's buttons):

```
mqttui publish autonomous/desk1/set preset=1
mqttui publish autonomous/desk1/set height=74
```

### State

The module publishes the current desk height in centimeters every second on topic `autonomous/<hostname>/state`

## TODO

* Runtime configuration
* Home Assistant MQTT discovery
