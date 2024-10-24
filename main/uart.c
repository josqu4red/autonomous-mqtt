#include "uart.h"

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

    ESP_ERROR_CHECK(uart_driver_install(UART_PORT, BUF_SIZE * 2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_PORT, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT, UART_TXD, UART_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_set_mode(UART_PORT, UART_MODE_RS485_HALF_DUPLEX));
}

esp_err_t uart_read(uint8_t* data, int len) {
    int bytes_read = uart_read_bytes(UART_PORT, data, len, PACKET_READ_TICS);
    if(bytes_read == -1) {
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t uart_write(uint8_t* data, int len) {
    uart_write_bytes(UART_PORT, (char*)data, len);
    return uart_wait_tx_done(UART_PORT, 100);
}
