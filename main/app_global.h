#ifndef APP_GLOBAL_H
#define APP_GLOBAL_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <driver/gpio.h>
#include <esp_event_loop.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <esp_log.h>
#include <esp_attr.h>
#include <esp_bt.h>
#include <esp_bt_main.h>
#include <esp_gap_ble_api.h>
#include <nvs_flash.h>

#include <lwip/err.h>
#include <apps/sntp/sntp.h>

#include "sdkconfig.h"

#define TAG "APP"

volatile extern EventGroupHandle_t g_app_evt;
volatile extern bool g_clock_warning;
volatile extern bool g_led_signal;

// App event MSG bits
#define APP_EVT_WIFI_CONNECTED 	0x00000001
#define APP_EVT_MQTT_CONNECTED 	0x00000002
#define APP_EVT_MQTT_READY 		0x00000004

#endif // APP_GLOBAL_H
