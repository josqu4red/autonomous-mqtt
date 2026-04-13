#ifndef AUTONOMOUS_DESK
#define AUTONOMOUS_DESK

#include "uart.h"
#include <stdatomic.h>
#include <stdint.h>

#define WRITE_BUF 5
#define SEND_HEADER1 0xD8
#define SEND_HEADER2 0x66
#define POLL_INTERVAL        pdMS_TO_TICKS(20)
#define CMD_INTERVAL         pdMS_TO_TICKS(150)
#define PRESET_WARMUP_TICKS  (pdMS_TO_TICKS(2000) / POLL_INTERVAL)

typedef enum {
  button_start = 0x00,
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

typedef _Atomic uint8_t shared_position_t;

static const button_t presets[4] = {button_1, button_2, button_3, button_4};

#define position_threshold CONFIG_AUTONOMOUS_POSITION_THRESHOLD
#define idle_threshold     CONFIG_AUTONOMOUS_IDLE_THRESHOLD
#define move_timeout_ticks CONFIG_AUTONOMOUS_MOVE_TIMEOUT


typedef enum {
  CMD_HEIGHT,
  CMD_PRESET,
} desk_cmd_type_t;

typedef struct {
  desk_cmd_type_t type;
  uint8_t value;
} desk_cmd_t;

bool valid_position(position_t position);

void desk_cancel(void);

void go_to_height(position_t desired, shared_position_t *position);

void go_to_preset(button_t preset, shared_position_t *position);

void desk_task(void *arg);

#endif // AUTONOMOUS_DESK
