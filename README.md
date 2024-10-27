# autonomous-mqtt - Autonomous desk MQTT remote control

Control your Autonomous desk through MQTT.

Based on [Stefichen5/AutonomousControl](https://github.com/Stefichen5/AutonomousControl) analysis of the command protocol.

### Prerequisites

Using Nix, just run `nix-shell` (or use [nix-direnv](https://github.com/nix-community/nix-direnv)) to get all dependencies set up.

Otherwise follow [ESP-IDF installation procedure](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html).

### Configure the project
```
idf.py menuconfig
```

There is an Autonomous section with relevant settings (see [project config](main/Kconfig.projbuild)).

### Build and Flash
Build the project and flash it to the board, then run monitor tool to view serial output:
```
idf.py -p PORT flash monitor
```
