# UART RS485 Echo Example

```
  ------------------------------------------------------------------------------------------------------------------------------
  |  UART Interface       | #define            | Default pin for ESP32 | Default pins for others   | External RS485 Driver Pin |
  | ----------------------|--------------------|-----------------------|---------------------------|---------------------------|
  | Transmit Data (TxD)   | CONFIG_MB_UART_TXD | GPIO23                | GPIO9                     | DI                        |
  | Receive Data (RxD)    | CONFIG_MB_UART_RXD | GPIO22                | GPIO8                     | RO                        |
  | Request To Send (RTS) | CONFIG_MB_UART_RTS | GPIO18                | GPIO10                    | ~RE/DE                    |
  | Ground                | n/a                | GND                   | GND                       | GND                       |
  ------------------------------------------------------------------------------------------------------------------------------
```
Note: Each target chip has different GPIO pins available for UART connection. Please refer to UART documentation for selected target for more information.

### Configure the project
```
idf.py menuconfig
```

### Build and Flash
Build the project and flash it to the board, then run monitor tool to view serial output:
```
idf.py -p PORT flash monitor
```
