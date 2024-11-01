#ifndef AUTONOMOUS_DESK
#define AUTONOMOUS_DESK

#include <stdint.h>
#include "uart.h"

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

typedef enum {
    position_low = 0x42,
    position_high = 0x83,
    position_error = 0xFF,
} limit_position_t;

typedef uint8_t position_t;

static const button_t presets[4] = {button_1, button_2, button_3, button_4};

static const uint8_t position_threshold = 2;
static const uint8_t idle_threshold = 50;
static const uint32_t send_delay = 100 / portTICK_PERIOD_MS;

static const char* data_topic = "autonomous/desk1/data";
static const char* cmd_height_topic = "autonomous/desk1/command/height";
static const char* cmd_preset_topic = "autonomous/desk1/command/preset";

void go_to_height(position_t desired, position_t* position);

void go_to_preset(uint8_t preset, position_t* position);

#endif // AUTONOMOUS_DESK
