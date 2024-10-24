#include "driver/uart.h"
#include "esp_log.h"

#define UART_PORT         (CONFIG_AUTONOMOUS_UART_NUM)
#define UART_TXD          (CONFIG_AUTONOMOUS_UART_TXD)
#define UART_RXD          (CONFIG_AUTONOMOUS_UART_RXD)

#define BUF_SIZE          (127)
#define BAUD_RATE         (9600)

#define PACKET_READ_TICS  (80 / portTICK_PERIOD_MS)
#define READ_BUF          (6)
#define WRITE_BUF         (5)

const uint8_t recv_header1 = 0x98;
const uint8_t recv_header2a = 0x0;
const uint8_t recv_header2b = 0x3;

const uint8_t lo_position = 0x42;
const uint8_t hi_position = 0x83;
// 10th of inches (:
const int step = 4;
const int min_height = 259;
const int max_height = 515;


uint8_t get_position(uint8_t *buf) {
  if(buf[0] != recv_header1) { return 0; };
  if(buf[1] != recv_header1) { return 0; };
  if(buf[2] != recv_header2a && buf[2] != recv_header2b) { return 0; };
  if(buf[3] != recv_header2a && buf[3] != recv_header2b) { return 0; };
  if(buf[4] != buf[5]) { return 0; };
  return buf[4] - lo_position;
}

float get_height(uint8_t position) {
  return (float)(min_height + (position * step)) / 10;
}

void init(void) {
    const int uart_num = UART_PORT;
    const uart_config_t uart_config = {
        .baud_rate = BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_ERROR_CHECK(uart_driver_install(uart_num, BUF_SIZE * 2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(uart_num, UART_TXD, UART_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_set_mode(uart_num, UART_MODE_RS485_HALF_DUPLEX));
}

static void uart_read(void *arg) {
    static const char *tag = "uart_read";
    uint8_t data[READ_BUF] = {0};

    esp_log_level_set(tag, ESP_LOG_INFO);
    ESP_LOGI(tag, "UART start receive loop.\r");

    while (1) {
        if (uart_read_bytes(UART_PORT, data, READ_BUF, PACKET_READ_TICS) > 0) {
            uint8_t position = get_position(data);
            float height = get_height(position);
            printf("Receiving: ");
            for (int i = 0; i < READ_BUF; i++) {
                printf("0x%X ", (uint8_t)data[i]);
            }
            printf("- height: %f\n", height);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void send_data(uint8_t *data) {
    uart_write_bytes(UART_PORT, (char*)data, WRITE_BUF);
    printf("Sending: ");
    for (int i = 0; i < WRITE_BUF; i++) {
        printf("0x%X ", (uint8_t)data[i]);
    }
    printf("\n");
    ESP_ERROR_CHECK(uart_wait_tx_done(UART_PORT, 100));
}

static void uart_write(void *arg) {
    static const char *tag = "uart_write";
    uint8_t data_init[WRITE_BUF] = { 0xD8, 0xD8, 0x66, 0x00, 0x00 };
    //uint8_t data[WRITE_BUF] = { 0xD8, 0xD8, 0x66, 0x01, 0x01 }; // down
    //uint8_t data[WRITE_BUF] = { 0xD8, 0xD8, 0x66, 0x02, 0x02 }; // up
    uint8_t data[WRITE_BUF] = { 0xD8, 0xD8, 0x66, 0x04, 0x04 }; // 1
    //uint8_t data[WRITE_BUF] = { 0xD8, 0xD8, 0x66, 0x08, 0x08 }; // 2

    esp_log_level_set(tag, ESP_LOG_INFO);
    ESP_LOGI(tag, "UART start transmit loop.\r");

    send_data(data_init);
    for(int i = 0; i < 1; i++) {
      vTaskDelay(50 / portTICK_PERIOD_MS);
      send_data(data);
    }

    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void app_main(void) {
    init();
    xTaskCreate(uart_read, "uart_read_task", 2048, NULL, 10, NULL);
    xTaskCreate(uart_write, "uart_write_task", 2048, NULL, configMAX_PRIORITIES - 2, NULL);
}
