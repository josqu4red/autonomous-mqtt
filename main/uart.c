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

static void build_command(uint8_t* buf, uint8_t button) {
    buf[0] = send_hdr1;
    buf[1] = send_hdr1;
    buf[2] = send_hdr2;
    buf[3] = button;
    buf[4] = button;
}

static uint8_t decode_position(uint8_t *buf) {
    if(buf[0] != recv_hdr1) { return err_position; };
    if(buf[1] != recv_hdr1) { return err_position; };
    if(buf[2] != recv_hdr2a && buf[2] != recv_hdr2b) { return err_position; };
    if(buf[3] != recv_hdr2a && buf[3] != recv_hdr2b) { return err_position; };
    if(buf[4] != buf[5]) { return err_position; };
    return buf[4];
}

void send_command(uint8_t command) {
    uint8_t data[WRITE_BUF] = {0};
    build_command(data, command);
    uart_write_bytes(UART_PORT, (char*)data, WRITE_BUF);
    uart_wait_tx_done(UART_PORT, 100);
}

void uart_event_handler(void *data) {
    uart_event_t event;
    uint8_t* arg = (uint8_t*) data;
    uint8_t msg[READ_BUF] = {0};
    for (;;) {
        if (xQueueReceive(uart0_queue, (void*)&event, (TickType_t)portMAX_DELAY)) {
            switch (event.type) {
            case UART_DATA:
                uart_read_bytes(UART_PORT, msg, event.size, portMAX_DELAY);
                ESP_LOGD(tag, "UART_DATA: 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X", msg[0], msg[1], msg[2], msg[3], msg[4], msg[5]);
                *arg = decode_position(msg);
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
