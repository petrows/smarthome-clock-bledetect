#ifndef APP_GLOBAL_H
#define APP_GLOBAL_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <esp_event_loop.h>

extern EventGroupHandle_t g_app_evt;

// App event MSG bits
#define APP_EVT_WIFI_CONNECTED 	0x00000001
#define APP_EVT_MQTT_CONNECTED 	0x00000002
#define APP_EVT_MQTT_READY 		0x00000004

#endif // APP_GLOBAL_H
