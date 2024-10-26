#include "esp_log.h"
#include "uart.h"

static char* tag = "uart";
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

void uart_event_handler(void *data) {
    uart_event_t event;
    for (;;) {
        if (xQueueReceive(uart0_queue, (void *)&event, (TickType_t)portMAX_DELAY)) {
            switch (event.type) {
            case UART_DATA:
                uart_read_bytes(UART_PORT, (uint8_t*) data, event.size, portMAX_DELAY);
                uint8_t* d = (uint8_t*) data;
                ESP_LOGD(tag, "UART_DATA: 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X", d[0], d[1], d[2], d[3], d[4], d[5]);
                break;
            case UART_FIFO_OVF:
                // If fifo overflow happened, you should consider adding flow control for your application.
                // The ISR has already reset the rx FIFO,
                // As an example, we directly flush the rx buffer here in order to read more data.
                ESP_LOGD(tag, "FIFO overflow");
                uart_flush_input(UART_PORT);
                xQueueReset(uart0_queue);
                break;
            case UART_BUFFER_FULL:
                // If buffer full happened, you should consider increasing your buffer size
                // As an example, we directly flush the rx buffer here in order to read more data.
                ESP_LOGD(tag, "RX buffer full");
                uart_flush_input(UART_PORT);
                xQueueReset(uart0_queue);
                break;
            default:
                ESP_LOGD(tag, "uart event type: %d", event.type);
                break;
            }
        }
    }
    vTaskDelete(NULL);
}
