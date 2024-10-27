#ifndef AUTONOMOUS_DESK
#define AUTONOMOUS_DESK

#include <stdint.h>
#include "mqtt.h"
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

void desk_mqtt_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

#endif // AUTONOMOUS_DESK
