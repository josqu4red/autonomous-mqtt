#include "esp_log.h"
#include "nvs_flash.h"
#include "desk.h"
#include "uart.h"
#include "wifi.h"

static const char *TAG = "main";

void read_pos(void* position) {
    uint8_t* pos = (uint8_t*) position;
    for (;;) {
        for (int i = 0; i < READ_BUF; i++) {
            printf("0x%X ", pos[i]);
        }
        printf("\n");
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

    uint8_t position[READ_BUF] = {0};
    xTaskCreate(read_pos, "read_pos_loop", 2048, (void*) position, 10, NULL);
    xTaskCreate(uart_event_handler, "uart_event_handler", 2048, (void*) position, 10, NULL);
}
