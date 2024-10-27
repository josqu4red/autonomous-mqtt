#ifndef AUTONOMOUS_MQTT
#define AUTONOMOUS_MQTT

#include "mqtt_client.h"

#define MQTT_BROKER_URL (CONFIG_AUTONOMOUS_MQTT_BROKER_URL)

esp_mqtt_client_handle_t mqtt_init(void);

#endif // AUTONOMOUS_MQTT
