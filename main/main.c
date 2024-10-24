#include "nvs_flash.h"
#include "uart.h"
#include "wifi.h"
#include "esp_log.h"

static const char *TAG = "main";

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
  return (float)(min_height + (position * step)) / 10; }

void read_position(void *arg) {
    uint8_t data[READ_BUF] = {0};

    while (1) {
        if (uart_read(data, READ_BUF) == ESP_OK) {
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
    printf("Sending: ");
    for (int i = 0; i < WRITE_BUF; i++) {
        printf("0x%X ", (uint8_t)data[i]);
    }
    printf("\n");
    uart_write(data, WRITE_BUF);
}

void send_command(void *arg) {
    uint8_t data_init[WRITE_BUF] = { 0xD8, 0xD8, 0x66, 0x00, 0x00 };
    //uint8_t data[WRITE_BUF] = { 0xD8, 0xD8, 0x66, 0x01, 0x01 }; // down
    //uint8_t data[WRITE_BUF] = { 0xD8, 0xD8, 0x66, 0x02, 0x02 }; // up
    uint8_t data[WRITE_BUF] = { 0xD8, 0xD8, 0x66, 0x04, 0x04 }; // 1
    //uint8_t data[WRITE_BUF] = { 0xD8, 0xD8, 0x66, 0x08, 0x08 }; // 2

    send_data(data_init);
    for(int i = 0; i < 1; i++) {
      vTaskDelay(50 / portTICK_PERIOD_MS);
      send_data(data);
    }

    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing NV storage");
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_LOGI(TAG, "Initializing WiFi");
    wifi_init();

    ESP_LOGI(TAG, "Initializing UART");
    uart_init();

    xTaskCreate(read_position, "uart_read_task", 2048, NULL, 10, NULL);
    xTaskCreate(send_command, "uart_write_task", 2048, NULL, configMAX_PRIORITIES - 2, NULL);
}
