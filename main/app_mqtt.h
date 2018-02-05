
#include "app_global.h"
#include <mqtt.h>

extern mqtt_client * g_mqtt;
extern bool g_mqtt_connected;

bool app_mqtt_init(void);
bool app_mqtt_wait(void);
bool app_mqtt_stop(void);
bool app_mqtt_clear(void);
