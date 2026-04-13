#include "desk.h"
#include "esp_log.h"
#include "mqtt.h"
#include "nvs_flash.h"
#include "uart.h"
#include "wifi.h"
#include <stdio.h>
#include <string.h>

static const char *tag = "main";
static esp_mqtt_client_handle_t mqtt_cli;
QueueHandle_t desk_cmd_queue;

void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                        int32_t event_id, void *event_data) {
  esp_mqtt_event_handle_t event = event_data;
  esp_mqtt_client_handle_t client = event->client;

  switch ((esp_mqtt_event_id_t)event_id) {
  case MQTT_EVENT_CONNECTED:
    ESP_LOGD(tag, "Connected to broker");
    esp_mqtt_client_subscribe(client, command_topic, 2);
    break;
  case MQTT_EVENT_SUBSCRIBED:
    ESP_LOGD(tag, "Subscribed to topic");
    break;
  case MQTT_EVENT_DATA: {
    ESP_LOGD(tag, "Received message %.*s of length %d", event->data_len,
             event->data, event->data_len);
    if (event->data_len >= 16) {
      ESP_LOGW(tag, "Payload too long (%d bytes), ignoring", event->data_len);
      break;
    }
    char payload[16];
    memcpy(payload, event->data, event->data_len);
    payload[event->data_len] = '\0';
    char action[8];
    int value = 0;
    if (sscanf(payload, "%7[^=]=%d", action, &value) == 2) {
      if (strcmp("height", action) == 0) {
        if (!valid_position(value)) {
          ESP_LOGW(tag, "Invalid height '%d'", value);
          break;
        }
        desk_cancel();
        desk_cmd_t cmd = {.type = CMD_HEIGHT, .value = (uint8_t)value};
        xQueueOverwrite(desk_cmd_queue, &cmd);
      } else if (strcmp("preset", action) == 0) {
        if ((value < 1) || (value > (sizeof(presets) / sizeof(*presets)))) {
          ESP_LOGW(tag, "Invalid preset '%d'", value);
          break;
        }
        desk_cancel();
        desk_cmd_t cmd = {.type = CMD_PRESET, .value = (uint8_t)value};
        xQueueOverwrite(desk_cmd_queue, &cmd);
      } else {
        ESP_LOGW(tag, "Invalid action '%s'", action);
      }
    } else {
      ESP_LOGW(tag, "Invalid message '%s'", payload);
    }
    break;
  }
  default:
    break;
  }
}

void mqtt_publish_position(void *data) {
  char position_c[4];
  shared_position_t *shared = (shared_position_t *)data;
  for (;;) {
    position_t pos = atomic_load(shared);
    if (valid_position(pos)) {
      snprintf(position_c, sizeof(position_c), "%u", pos);
      esp_mqtt_client_enqueue(mqtt_cli, state_topic, position_c, 0, 1, 0,
                              false);
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
  vTaskDelete(NULL);
}

void app_main(void) {
  esp_log_level_set("*", ESP_LOG_INFO);
  esp_log_level_set("main", ESP_LOG_DEBUG);
  esp_log_level_set("desk", ESP_LOG_DEBUG);

  shared_position_t position = 0;

  ESP_LOGI(tag, "Initializing NV storage");
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  ESP_ERROR_CHECK(esp_event_loop_create_default());

  ESP_LOGI(tag, "Initializing WiFi");
  wifi_init();

  ESP_LOGI(tag, "Starting UART event handler");
  uart_init();
  ESP_ERROR_CHECK(xTaskCreate(uart_event_handler, "uart_event_handler", 2048,
                              (void *)&position, 5, NULL) != pdPASS
                      ? ESP_FAIL
                      : ESP_OK);

  ESP_LOGI(tag, "Starting desk task");
  desk_cmd_queue = xQueueCreate(1, sizeof(desk_cmd_t));
  ESP_ERROR_CHECK(desk_cmd_queue == NULL ? ESP_FAIL : ESP_OK);
  ESP_ERROR_CHECK(xTaskCreate(desk_task, "desk_task", 4096, (void *)&position,
                              4, NULL) != pdPASS
                      ? ESP_FAIL
                      : ESP_OK);

  ESP_LOGI(tag, "Starting MQTT event handler");
  mqtt_cli = mqtt_init();
  ESP_ERROR_CHECK(mqtt_cli == NULL ? ESP_FAIL : ESP_OK);
  esp_mqtt_client_register_event(mqtt_cli, MQTT_EVENT_ANY, mqtt_event_handler,
                                 NULL);
  esp_mqtt_client_start(mqtt_cli);

  ESP_LOGI(tag, "Starting position exporter");
  ESP_ERROR_CHECK(xTaskCreate(mqtt_publish_position, "mqtt_publish_position",
                              2048, (void *)&position, 2, NULL) != pdPASS
                      ? ESP_FAIL
                      : ESP_OK);
}
