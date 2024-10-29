#include <stdio.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "desk.h"
#include "mqtt.h"
#include "uart.h"
#include "wifi.h"

static const char *tag = "main";
static esp_mqtt_client_handle_t mqtt_cli;

void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    uint8_t* position = (uint8_t*) handler_args;
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGD(tag, "Connected to broker");
        esp_mqtt_client_subscribe(client, cmd_height_topic, 2);
        esp_mqtt_client_subscribe(client, cmd_preset_topic, 2);
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGD(tag, "Subscribed to topic");
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGD(tag, "Received message %.*s of length %d on topic %s", event->data_len, event->data, event->data_len, event->topic);
        char* payload = event->data;
        payload[event->data_len] = '\0';
        int value = 0;
        if (sscanf(payload, "%d", &value) == 1) {
            ESP_LOGD(tag, "Received data: %d\n", value);
            if (strcmp(cmd_height_topic, event->topic) == 0) {
                if (!valid_position(value)) {
                    ESP_LOGW(tag, "Got invalid height %d\n", value);
                    break;
                }
                go_to_height((position_t)value, position);
            } else if (strcmp(cmd_preset_topic, event->topic) == 0) {
                if ((value < 1) || (value > sizeof(presets))) {
                    ESP_LOGW(tag, "Got invalid preset %d\n", value);
                    break;
                }
                go_to_preset((uint8_t)value, position);
            }
        } else {
            ESP_LOGD(tag, "Message not matched");
        }
        break;
    default:
        break;
    }
}

void mqtt_publish_position(void* data) {
    char position_c[4];
    for (;;) {
        position_t* position = (uint8_t*) data;
        if (valid_position(*position)) {
            sprintf(position_c, "%u", *position);
            esp_mqtt_client_enqueue(mqtt_cli, data_topic, position_c, 0, 1, 0, false);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
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

    ESP_LOGI(tag, "Starting UART event handler");
    uart_init();
    xTaskCreate(uart_event_handler, "uart_event_handler", 2048, (void*) position, 2, NULL);

    ESP_LOGI(tag, "Starting MQTT event handler");
    mqtt_cli = mqtt_init();
    esp_mqtt_client_register_event(mqtt_cli, MQTT_EVENT_ANY, mqtt_event_handler, (void*) position);
    esp_mqtt_client_start(mqtt_cli);

    ESP_LOGI(tag, "Starting position exporter");
    xTaskCreate(mqtt_publish_position, "mqtt_publish_position", 2048, (void*) position, 5, NULL);
}
