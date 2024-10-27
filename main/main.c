#include <stdio.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "desk.h"
#include "mqtt.h"
#include "uart.h"
#include "wifi.h"

static const char *tag = "main";
static esp_mqtt_client_handle_t mqtt_cli;
static const char* data_topic = "autonomous/desk1/data";

void read_pos(void* data) {
    char position_c[4];
    for (;;) {
        position_t* position = (uint8_t*) data;
        sprintf(position_c, "%u", *position);
        esp_mqtt_client_enqueue(mqtt_cli, data_topic, position_c, 0, 1, 0, false);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void app_main(void) {
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("main", ESP_LOG_DEBUG);
    esp_log_level_set("desk", ESP_LOG_DEBUG);

    position_t position[1] = {0};

    ESP_LOGI(tag, "Initializing NV storage");
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_LOGI(tag, "Initializing WiFi");
    wifi_init();

    ESP_LOGI(tag, "Initializing MQTT");
    mqtt_cli = mqtt_init();

    ESP_LOGI(tag, "Initializing UART");
    uart_init();

    ESP_LOGI(tag, "Starting UART event handler");
    xTaskCreate(uart_event_handler, "uart_event_handler", 2048, (void*) position, 10, NULL);
    ESP_LOGI(tag, "Starting MQTT event handler");
    esp_mqtt_client_register_event(mqtt_cli, ESP_EVENT_ANY_ID, desk_mqtt_handler, (void*) position);
    esp_mqtt_client_start(mqtt_cli);
    ESP_LOGI(tag, "Starting position exporter");
    xTaskCreate(read_pos, "read_pos_loop", 2048, (void*) position, 10, NULL);
}
