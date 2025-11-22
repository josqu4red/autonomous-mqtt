#ifndef AUTONOMOUS_WIFI
#define AUTONOMOUS_WIFI

#include "esp_wifi.h"

/* The event group allows multiple bits for each event, but we only care about
 * two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

void wifi_init(void);

#endif // AUTONOMOUS_WIFI
