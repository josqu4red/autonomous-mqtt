#include "esp_log.h"
#include "desk.h"

static const char *tag = "desk";

static const char* cmd_height_topic = "autonomous/desk1/command/height";
static const char* cmd_preset_topic = "autonomous/desk1/command/preset";

static void go_to_height(position_t desired, position_t* position) {
    ESP_LOGI(tag, "Moving to height %d\n", desired);
    position_t current = *position;
    bool done = false;
    if (desired == current) {
        return;
    }

    button_t direction;
    if (desired < current) {
        direction = button_down;
    } else {
        direction = button_up;
    }

    send_command(button_none);

    while(!done) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
        send_command(direction);
        current = *position;
        ESP_LOGD(tag, "position: %d\n", current);

        if (direction == button_up) {
          ESP_LOGD(tag, "current %d >= desired %d\n", current, desired);
          done = (current >= desired - position_threshold);
        } else {
          ESP_LOGD(tag, "current %d <= desired %d\n", current, desired);
          done = (current <= desired + position_threshold);
        }
    }
}

static void go_to_preset(uint8_t preset, position_t* position) {
    ESP_LOGI(tag, "Moving to preset %d\n", preset);
    button_t button = presets[preset-1];
    position_t last = *position;
    bool done = false;
    int idle = 0;

    send_command(button_none);

    while(!done) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
        send_command(button);
        position_t current = *position;
        ESP_LOGD(tag, "position: 0x%X; idle:%d\n", current, idle);

        if(last == current) {
            idle++;
            if (idle >= idle_threshold) {
                done = true;
            }
        } else {
            idle = 0;
            last = current;
        }
    }
}

void desk_mqtt_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
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
                if ((value <= position_low) || (value >= position_high)) {
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
