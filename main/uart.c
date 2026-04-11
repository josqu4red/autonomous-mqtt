#include <stdatomic.h>
#include <string.h>
#include "esp_log.h"
#include "uart.h"

static const char* tag = "uart";

static const uint8_t recv_hdr1    = 0x98;
static const uint8_t recv_hdr2a   = 0x00;
static const uint8_t recv_hdr2b   = 0x03;
static const uint8_t err_position = 0xFF;
static QueueHandle_t uart0_queue;

void uart_init(void) {
    const uart_config_t uart_config = {
        .baud_rate = BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_ERROR_CHECK(uart_driver_install(UART_PORT, BUF_SIZE * 2, 0, 20, &uart0_queue, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_PORT, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT, UART_TXD, UART_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_set_mode(UART_PORT, UART_MODE_RS485_HALF_DUPLEX));
}

esp_err_t uart_write(uint8_t* data, int len) {
    uart_write_bytes(UART_PORT, (char*)data, len);
    return uart_wait_tx_done(UART_PORT, 100);
}

static uint8_t decode_position(uint8_t *buf) {
    if(buf[0] != recv_hdr1) { return err_position; };
    if(buf[1] != recv_hdr1) { return err_position; };
    if(buf[2] != recv_hdr2a && buf[2] != recv_hdr2b) { return err_position; };
    if(buf[3] != recv_hdr2a && buf[3] != recv_hdr2b) { return err_position; };
    if(buf[4] != buf[5]) { return err_position; };
    return buf[4];
}

void uart_event_handler(void *data) {
    uart_event_t event;
    _Atomic uint8_t* arg = (_Atomic uint8_t*) data;
    uint8_t buf[FRAME_BUF] = {0};
    int buf_len = 0;

    for (;;) {
        if (!xQueueReceive(uart0_queue, (void*)&event, (TickType_t)portMAX_DELAY)) {
            continue;
        }
        switch (event.type) {
        case UART_DATA: {
            int len = uart_read_bytes(UART_PORT, buf + buf_len, sizeof(buf) - buf_len, 0);
            buf_len += len;

            int i = 0;
            while (i <= buf_len - READ_BUF) {
                if (buf[i] == recv_hdr1 && buf[i+1] == recv_hdr1) {
                    ESP_LOGD(tag, "UART_DATA: 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X",
                             buf[i], buf[i+1], buf[i+2], buf[i+3], buf[i+4], buf[i+5]);
                    atomic_store(arg, decode_position(buf + i));
                    i += READ_BUF;
                } else {
                    i++;
                }
            }
            buf_len -= i;
            memmove(buf, buf + i, buf_len);
            break;
        }
        case UART_FIFO_OVF:
            ESP_LOGD(tag, "FIFO overflow");
            uart_flush_input(UART_PORT);
            xQueueReset(uart0_queue);
            buf_len = 0;
            break;
        case UART_BUFFER_FULL:
            ESP_LOGD(tag, "RX buffer full");
            uart_flush_input(UART_PORT);
            xQueueReset(uart0_queue);
            buf_len = 0;
            break;
        default:
            ESP_LOGD(tag, "uart event type: %d", event.type);
            break;
        }
    }
    vTaskDelete(NULL);
}
