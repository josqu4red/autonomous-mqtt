#ifndef AUTONOMOUS_DESK
#define AUTONOMOUS_DESK

#include <stdint.h>

typedef enum {
    button_none = 0x00,
    button_down = 0x01,
    button_up = 0x02,
    button_1 = 0x04,
    button_2 = 0x08,
    button_3 = 0x10,
    button_4 = 0x20,
    button_M = 0x40,
} button_t;

static const uint8_t recv_hdr1 = 0x98;
static const uint8_t recv_hdr2a = 0x0;
static const uint8_t recv_hdr2b = 0x3;

static const uint8_t send_hdr1 = 0xD8;
static const uint8_t send_hdr2 = 0x66;

static const uint8_t low_position = 0x42;
static const uint8_t high_position = 0x83;
static const uint8_t err_position = 0xFF;
// 10th of inches (:
static const int step = 4;
static const int min_height = 259;
static const int max_height = 515;

uint8_t read_position(void);

void loop_position(void *arg);

void go_to_height(float height);

void go_to_preset(button_t button);

#endif // AUTONOMOUS_DESK
