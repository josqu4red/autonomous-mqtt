#ifndef AUTONOMOUS_UART
#define AUTONOMOUS_UART

#include "driver/uart.h"

#define UART_PORT         (CONFIG_AUTONOMOUS_UART_NUM)
#define UART_TXD          (CONFIG_AUTONOMOUS_UART_TXD)
#define UART_RXD          (CONFIG_AUTONOMOUS_UART_RXD)

#define BUF_SIZE          (127)
#define BAUD_RATE         (9600)

#define READ_BUF          (6)
#define WRITE_BUF         (5)

static const uint8_t recv_hdr1 = 0x98;
static const uint8_t recv_hdr2a = 0x0;
static const uint8_t recv_hdr2b = 0x3;
static const uint8_t err_position = 0xFF;

static const uint8_t send_hdr1 = 0xD8;
static const uint8_t send_hdr2 = 0x66;

void uart_init(void);

void send_command(uint8_t command);

void uart_event_handler(void* data);

#endif // AUTONOMOUS_UART
