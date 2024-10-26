#include "desk.h"
#include "uart.h"

static uint8_t decode_position(uint8_t *buf) {
    if(buf[0] != recv_hdr1) { return err_position; };
    if(buf[1] != recv_hdr1) { return err_position; };
    if(buf[2] != recv_hdr2a && buf[2] != recv_hdr2b) { return err_position; };
    if(buf[3] != recv_hdr2a && buf[3] != recv_hdr2b) { return err_position; };
    if(buf[4] != buf[5]) { return err_position; };
    return buf[4] - low_position;
}

static void build_command(uint8_t* buf, button_t button) {
    buf[0] = send_hdr1;
    buf[1] = send_hdr1;
    buf[2] = send_hdr2;
    buf[3] = button;
    buf[4] = button;
}

static float position_to_height(uint8_t position) {
    return (float)(min_height + (position * step)) / 10;
}

// static uint8_t height_to_position(float height) {
//     return (height * 10 - min_height) / step;
// }

uint8_t read_position(void) {
    uint8_t data[READ_BUF] = {0};
    uart_read(data, READ_BUF);
    return decode_position(data);
}

void loop_position(void *arg) {
    uint8_t data[READ_BUF] = {0};

    while (1) {
        if (uart_read(data, READ_BUF) == ESP_OK) {
            uint8_t position = decode_position(data);
            float height = position_to_height(position);
            printf("Receiving: ");
            for (int i = 0; i < READ_BUF; i++) {
                printf("0x%X ", (uint8_t)data[i]);
            }
            printf("- height: %f\n", height);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void send_command(button_t button) {
    uint8_t data[WRITE_BUF] = {0};
    build_command(data, button);
    uart_write(data, WRITE_BUF);
}

void go_to_height(float height) {
  bool done = false;
  while(!done) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void go_to_preset(button_t button) {
  vTaskDelay(50 / portTICK_PERIOD_MS);
  send_command(button_none);
  vTaskDelay(50 / portTICK_PERIOD_MS);
  send_command(button);
  //bool done = false;
  //while(!done) {
  //      vTaskDelay(1000 / portTICK_PERIOD_MS);
  //}
}
