#include "esp_log.h"
#include "desk.h"

static const char *tag = "desk";

static void build_command(uint8_t* buf, button_t button) {
    buf[0] = SEND_HEADER1;
    buf[1] = SEND_HEADER1;
    buf[2] = SEND_HEADER2;
    buf[3] = button;
    buf[4] = button;
}

static void send_command(button_t command) {
    uint8_t data[WRITE_BUF] = {0};
    build_command(data, command);
    uart_write(data, WRITE_BUF);
}

bool valid_position(position_t position) {
    if ((position >= position_low) && (position <= position_high)) {
        return true;
    }
    return false;
}

void go_to_height(position_t desired, position_t* position) {
    ESP_LOGI(tag, "Moving to height %d", desired);
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

    send_command(button_start);

    while(!done) {
        vTaskDelay(SEND_DELAY);
        send_command(direction);
        current = *position;
        ESP_LOGD(tag, "position: %d", current);

        if (direction == button_up) {
          ESP_LOGD(tag, "current %d >= desired %d", current, desired);
          done = (current >= desired - position_threshold);
        } else {
          ESP_LOGD(tag, "current %d <= desired %d", current, desired);
          done = (current <= desired + position_threshold);
        }
    }
}

void go_to_preset(button_t preset, position_t* position) {
    ESP_LOGI(tag, "Moving to preset %d", preset);
    button_t button = presets[preset-1];
    position_t last = *position;
    bool done = false;
    int idle = 0;

    send_command(button_start);

    while(!done) {
        vTaskDelay(SEND_DELAY);
        send_command(button);
        position_t current = *position;
        ESP_LOGD(tag, "position: %d; idle:%d", current, idle);

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
