
#include "app_global.h"

#include <esp_mqtt.h>

extern bool g_mqtt_connected;

bool app_mqtt_init(void);
bool app_mqtt_start(void);
bool app_mqtt_stop(void);
void app_mqtt_send_message(const char * topic, const char * data, int len);
