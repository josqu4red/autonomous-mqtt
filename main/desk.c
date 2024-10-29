#include "esp_log.h"
#include "desk.h"

static const char *tag = "desk";

bool valid_position(position_t position) {
    if ((position >= position_low) && (position <= position_high)) {
        return true;
    }
    return false;
}

void go_to_height(position_t desired, position_t* position) {
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
        vTaskDelay(send_delay);
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

void go_to_preset(uint8_t preset, position_t* position) {
    ESP_LOGI(tag, "Moving to preset %d\n", preset);
    button_t button = presets[preset-1];
    position_t last = *position;
    bool done = false;
    int idle = 0;

    send_command(button_none);

    while(!done) {
        vTaskDelay(send_delay);
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
