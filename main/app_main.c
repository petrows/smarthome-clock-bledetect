
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

#include "global.h"

#include <tm1637.h>

void app_main() {
	ESP_ERROR_CHECK(nvs_flash_init());

	esp_err_t ret;
	wifi_event_group = xEventGroupCreate();
	ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

	setenv("TZ", "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00", 1);
	tzset();

	esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();

	ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
	ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BLE));
	ESP_ERROR_CHECK(esp_bluedroid_init());
	ESP_ERROR_CHECK(esp_bluedroid_enable());
	ESP_ERROR_CHECK(esp_ble_gap_register_callback(esp_gap_cb));

	// ESP_ERROR_CHECK( esp_ble_gap_start_scanning(0) );
	// ESP_ERROR_CHECK( esp_ble_gap_start_scanning(10) );

	//xTaskCreate(&blink_task, "blink_task", 4096, NULL, 5, NULL);
	xTaskCreate(&led_task, "led_task", 4096, NULL, 5, NULL);
	xTaskCreate(&ntp_task, "ntp_task", 4096, NULL, 5, NULL);
}
