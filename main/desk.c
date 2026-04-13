#include "desk.h"
#include "esp_log.h"
#include "freertos/queue.h"

static const char *tag = "desk";
static _Atomic bool cancel_flag = false;
extern QueueHandle_t desk_cmd_queue;

void desk_cancel(void) { atomic_store(&cancel_flag, true); }

static void build_command(uint8_t *buf, button_t button) {
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

void go_to_height(position_t desired, shared_position_t *shared) {
  ESP_LOGI(tag, "Moving to height %d", desired);
  atomic_store(&cancel_flag, false);
  position_t current = atomic_load(shared);
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
  send_command(direction);

  int ticks = 0;
  position_t initial = current;
  bool has_moved = false;
  TickType_t next_cmd = xTaskGetTickCount() + CMD_INTERVAL;

  while (!done && !atomic_load(&cancel_flag) && ticks < MOVE_TIMEOUT_TICKS) {
    vTaskDelay(POLL_INTERVAL);
    current = atomic_load(shared);
    ESP_LOGD(tag, "position: %d", current);
    ticks++;

    if (!has_moved) {
      has_moved = (direction == button_up) ? (current > initial) : (current < initial);
    }
    if (has_moved) {
      if (direction == button_up) {
        done = ((int16_t)current >= (int16_t)desired - (int16_t)POSITION_THRESHOLD);
      } else {
        done = ((int16_t)current <= (int16_t)desired + (int16_t)POSITION_THRESHOLD);
      }
    }
    if (done) break;

    if (xTaskGetTickCount() >= next_cmd) {
      send_command(direction);
      next_cmd = xTaskGetTickCount() + CMD_INTERVAL;
    }
  }
  send_command(button_start);
  if (ticks >= MOVE_TIMEOUT_TICKS) {
    ESP_LOGW(tag, "go_to_height timed out at position %d", current);
  }
}

void go_to_preset(button_t preset, shared_position_t *shared) {
  ESP_LOGI(tag, "Moving to preset %d", preset);
  atomic_store(&cancel_flag, false);
  button_t button = presets[preset - 1];
  position_t last = atomic_load(shared);
  bool done = false;
  int idle = 0;

  send_command(button_start);
  send_command(button);

  int ticks = 0;
  TickType_t next_cmd = xTaskGetTickCount() + CMD_INTERVAL;

  while (!done && !atomic_load(&cancel_flag) && ticks < MOVE_TIMEOUT_TICKS) {
    vTaskDelay(POLL_INTERVAL);
    position_t current = atomic_load(shared);
    ESP_LOGD(tag, "position: %d; idle:%d", current, idle);
    ticks++;

    if (ticks <= PRESET_WARMUP_TICKS) {
      last = current;
    } else if (last == current) {
      idle++;
      if (idle >= IDLE_THRESHOLD) {
        done = true;
      }
    } else {
      idle = 0;
      last = current;
    }
    if (done) break;

    if (xTaskGetTickCount() >= next_cmd) {
      send_command(button);
      next_cmd = xTaskGetTickCount() + CMD_INTERVAL;
    }
  }
  send_command(button_start);
  if (ticks >= MOVE_TIMEOUT_TICKS) {
    ESP_LOGW(tag, "go_to_preset timed out at position %d", last);
  }
}

void desk_task(void *arg) {
  shared_position_t *position = (shared_position_t *)arg;
  desk_cmd_t cmd;
  for (;;) {
    xQueueReceive(desk_cmd_queue, &cmd, portMAX_DELAY);
    if (cmd.type == CMD_HEIGHT) {
      go_to_height(cmd.value, position);
    } else if (cmd.type == CMD_PRESET) {
      go_to_preset(cmd.value, position);
    }
  }
  vTaskDelete(NULL);
}
