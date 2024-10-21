#include "driver/uart.h"
#include "esp_log.h"

#define TAG "autonomous"

#define TASK_STACK_SIZE   (CONFIG_AUTONOMOUS_TASK_STACK_SIZE)
#define TASK_PRIO         (10)

#define UART_PORT         (CONFIG_AUTONOMOUS_UART_NUM)
#define UART_TXD          (CONFIG_AUTONOMOUS_UART_TXD)
#define UART_RXD          (CONFIG_AUTONOMOUS_UART_RXD)
#define UART_RTS          (CONFIG_AUTONOMOUS_UART_RTS)
#define UART_CTS          (UART_PIN_NO_CHANGE)

#define BUF_SIZE          (127)
#define BAUD_RATE         (9600)

#define PACKET_READ_TICS  (80 / portTICK_PERIOD_MS)
#define READ_BUF          (6)

static void uart_read(void *arg) {
    const int uart_num = UART_PORT;
    uart_config_t uart_config = {
        .baud_rate = BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
        .source_clk = UART_SCLK_DEFAULT,
    };

    esp_log_level_set(TAG, ESP_LOG_INFO);

    ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(uart_num, UART_TXD, UART_RXD, UART_RTS, UART_CTS));
    ESP_ERROR_CHECK(uart_driver_install(uart_num, BUF_SIZE * 2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_set_mode(uart_num, UART_MODE_RS485_HALF_DUPLEX));

    uint8_t data[READ_BUF] = {0};
    //uint8_t read_header1[4] = { 0x98, 0x98, 0x0, 0x0 };
    //uint8_t read_header2[4] = { 0x98, 0x98, 0x3, 0x3 };
    //uint8_t write_header[3] = { 0xD8, 0xD8, 0x66 };

    ESP_LOGI(TAG, "UART start receive loop.\r");

    while (1) {
        if (uart_read_bytes(uart_num, data, READ_BUF, PACKET_READ_TICS) > 0) {
            for (int i = 0; i < READ_BUF; i++) {
                printf("0x%X ", (uint8_t)data[i]);
            }
            printf("\n");
        }
    }
    vTaskDelete(NULL);
}

void app_main(void) {
    xTaskCreate(uart_read, "uart_read_task", TASK_STACK_SIZE, NULL, TASK_PRIO, NULL);
}
