#include "esp_log.h"
#include "nvs_flash.h"
#include "desk.h"
#include "uart.h"
#include "wifi.h"

static const char *TAG = "main";

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

    button_t b = button_2;
    go_to_preset(b);
    xTaskCreate(loop_position, "uart_read_task", 2048, NULL, 10, NULL);
}
