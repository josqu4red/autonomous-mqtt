#include "esp_log.h"
#include "desk.h"

static const char *tag = "desk";

position_t decode_position(uint8_t *buf) {
    if(buf[0] != recv_hdr1) { return err_position; };
    if(buf[1] != recv_hdr1) { return err_position; };
    if(buf[2] != recv_hdr2a && buf[2] != recv_hdr2b) { return err_position; };
    if(buf[3] != recv_hdr2a && buf[3] != recv_hdr2b) { return err_position; };
    if(buf[4] != buf[5]) { return err_position; };
    return buf[4];
}

static void build_command(uint8_t* buf, button_t button) {
    buf[0] = send_hdr1;
    buf[1] = send_hdr1;
    buf[2] = send_hdr2;
    buf[3] = button;
    buf[4] = button;
}

static void send_command(button_t button) {
    uint8_t data[WRITE_BUF] = {0};
    build_command(data, button);
    uart_write(data, WRITE_BUF);
}

void go_to_height(position_t desired, uint8_t* position) {
    ESP_LOGI(tag, "Moving to height %d\n", desired);
    position_t current = decode_position(position);
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
        current = decode_position(position);
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

void go_to_preset(uint8_t preset, uint8_t* position) {
    ESP_LOGI(tag, "Moving to preset %d\n", preset);
    button_t button = presets[preset-1];
    position_t last = decode_position(position);
    bool done = false;
    int idle = 0;

    send_command(button_none);

    while(!done) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
        send_command(button);
        position_t current = decode_position(position);
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
