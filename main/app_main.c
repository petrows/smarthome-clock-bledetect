
#include "app_global.h"
#include "task_control.h"
#include "task_display.h"

volatile EventGroupHandle_t g_app_evt;
volatile bool g_clock_warning = false;
volatile bool g_led_signal = false;

static esp_err_t esp_event_handler(void *ctx, system_event_t *event)
{
	ESP_LOGI(TAG, "esp_event_handler event %d", event->event_id);

	switch (event->event_id)
	{
		case SYSTEM_EVENT_STA_START:
			esp_wifi_connect();
			break;
		case SYSTEM_EVENT_STA_GOT_IP:
			xEventGroupSetBits(g_app_evt, APP_EVT_WIFI_CONNECTED);
			break;
		case SYSTEM_EVENT_STA_DISCONNECTED:
			xEventGroupClearBits(g_app_evt, APP_EVT_WIFI_CONNECTED);
			esp_wifi_stop();
			esp_wifi_start();
			esp_wifi_connect();
			break;
		default:
			break;
	}

	return ESP_OK;
}

void app_main() {
	ESP_ERROR_CHECK(nvs_flash_init());

	g_app_evt = xEventGroupCreate();

	ESP_ERROR_CHECK(esp_event_loop_init(esp_event_handler, NULL)); // Global ESP event handler

	setenv("TZ", CONFIG_CLOCK_TIMEZONE, 1);
	tzset();

	xTaskCreate(&task_control, "task_control", 4096, NULL, 5, NULL);
	xTaskCreate(&task_display, "task_display", 4096, NULL, 5, NULL);
}
