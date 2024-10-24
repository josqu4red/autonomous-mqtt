#ifndef AUTONOMOUS_UART
#define AUTONOMOUS_UART

#include "driver/uart.h"

#define UART_PORT         (CONFIG_AUTONOMOUS_UART_NUM)
#define UART_TXD          (CONFIG_AUTONOMOUS_UART_TXD)
#define UART_RXD          (CONFIG_AUTONOMOUS_UART_RXD)

#define BUF_SIZE          (127)
#define BAUD_RATE         (9600)

#define PACKET_READ_TICS  (80 / portTICK_PERIOD_MS)
#define READ_BUF          (6)
#define WRITE_BUF         (5)

void uart_init(void);

esp_err_t uart_read(uint8_t* data, int len);

esp_err_t uart_write(uint8_t* data, int len);

#endif // AUTONOMOUS_UART
