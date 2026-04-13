#include "desk.h"
#include "esp_log.h"
#include "uart.h"

static char *tag = "uart";
static QueueHandle_t uart0_queue;

void uart_init(void) {
  const uart_config_t uart_config = {
      .baud_rate = BAUD_RATE,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .rx_flow_ctrl_thresh = 122,
      .source_clk = UART_SCLK_DEFAULT,
  };

  ESP_ERROR_CHECK(
      uart_driver_install(UART_PORT, BUF_SIZE * 2, 0, 20, &uart0_queue, 0));
  ESP_ERROR_CHECK(uart_param_config(UART_PORT, &uart_config));
  ESP_ERROR_CHECK(uart_set_pin(UART_PORT, UART_TXD, UART_RXD,
                               UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
  ESP_ERROR_CHECK(uart_set_mode(UART_PORT, UART_MODE_RS485_HALF_DUPLEX));
}

esp_err_t uart_write(uint8_t *data, int len) {
  uart_write_bytes(UART_PORT, (char *)data, len);
  return uart_wait_tx_done(UART_PORT, 100);
}

static uint8_t decode_position(uint8_t *buf) {
  if (buf[0] != recv_hdr1) {
    return err_position;
  };
  if (buf[1] != recv_hdr1) {
    return err_position;
  };
  if (buf[2] != recv_hdr2a && buf[2] != recv_hdr2b) {
    return err_position;
  };
  if (buf[3] != recv_hdr2a && buf[3] != recv_hdr2b) {
    return err_position;
  };
  if (buf[4] != buf[5]) {
    return err_position;
  };
  return buf[4];
}

void uart_event_handler(void *data) {
  uart_event_t event;
  shared_position_t *shared = (shared_position_t *)data;
  uint8_t msg[READ_BUF] = {0};
  for (;;) {
    if (xQueueReceive(uart0_queue, (void *)&event, (TickType_t)portMAX_DELAY)) {
      switch (event.type) {
      case UART_DATA:
        if (event.size != READ_BUF) {
          ESP_LOGW(tag, "Unexpected frame size %d, flushing", event.size);
          uart_flush_input(UART_PORT);
          break;
        }
        uart_read_bytes(UART_PORT, msg, READ_BUF, portMAX_DELAY);
        ESP_LOGD(tag, "UART_DATA: 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X", msg[0],
                 msg[1], msg[2], msg[3], msg[4], msg[5]);
        atomic_store(shared, decode_position(msg));
        break;
      case UART_FIFO_OVF:
        ESP_LOGW(tag, "FIFO overflow — data lost");
        uart_flush_input(UART_PORT);
        xQueueReset(uart0_queue);
        break;
      case UART_BUFFER_FULL:
        ESP_LOGW(tag, "RX buffer full — data lost");
        uart_flush_input(UART_PORT);
        xQueueReset(uart0_queue);
        break;
      default:
        ESP_LOGD(tag, "uart event type: %d", event.type);
        break;
      }
    }
  }
  vTaskDelete(NULL);
}
