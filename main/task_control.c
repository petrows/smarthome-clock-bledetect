#include "task_control.h"

static void initialize_sntp(void);
static void initialise_wifi(void);

static void initialize_sntp(void)
{
	ESP_LOGI(TAG, "Initializing SNTP");
	if (sntp_enabled()) {
		sntp_stop(); // Stop current instance
	}
	sntp_setoperatingmode(SNTP_OPMODE_POLL);
	sntp_setservername(0, "192.168.80.1");
	sntp_setservername(1, "192.168.80.2");
	sntp_setservername(2, "pool.ntp.org");

	sntp_init();
}

static void initialise_wifi(void)
{
	tcpip_adapter_init();
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
	wifi_config_t wifi_config;
	bzero(&wifi_config, sizeof(wifi_config));
	strcpy((char*) wifi_config.sta.ssid, CONFIG_WIFI_SSID);
	strcpy((char*) wifi_config.sta.password, CONFIG_WIFI_PASSWORD);
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
	ESP_ERROR_CHECK(esp_wifi_start());
}

void task_control(void *arg)
{
	EventBits_t evt_res;
	while (true)
	{
		// 0. Init WiFi and set time
		initialise_wifi();
		evt_res = xEventGroupWaitBits(g_app_evt, APP_EVT_WIFI_CONNECTED, false, true, (30 * 1000) / portTICK_PERIOD_MS );
		if (!(evt_res & APP_EVT_WIFI_CONNECTED)) {
			// Error!
			ESP_LOGE(TAG, "Wifi connection FAILED!");
			g_clock_warning = true;
			vTaskDelay( (30 * 1000) / portTICK_PERIOD_MS);
			continue; // Try to reconnect
		}

		// 1. Start NTP client
		initialize_sntp();

		// Wait for something
		// All ok?
		g_clock_warning = false;

		vTaskDelay( (10 * 1000) / portTICK_PERIOD_MS);

		// Stop wifi and go to scan mode
		ESP_ERROR_CHECK( esp_wifi_stop() );
		ESP_ERROR_CHECK( esp_ble_gap_start_scanning(10) );

		vTaskDelay( (10 * 1000) / portTICK_PERIOD_MS);
	}
}
