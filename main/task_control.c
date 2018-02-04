#include "task_control.h"

static bool app_sntp_init(void);
static bool app_sntp_wait(void);
static bool app_wifi_init(void);
static bool app_wifi_wait(void);
static bool app_mqtt_init(void);

bool app_sntp_init(void)
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
	return true;
}

bool app_sntp_wait(void)
{
	time_t now = 0;
	struct tm timeinfo;

	for (int ntp_retry=0; ntp_retry<10; ++ntp_retry)
	{
		vTaskDelay(2000 / portTICK_PERIOD_MS);

		time(&now);
		localtime_r(&now, &timeinfo);

		if (timeinfo.tm_year < (2016 - 1900)) {
			continue; // Wait...
		} else {
			return true; // Time is set
		}
	}

	return false;
}

bool app_wifi_init(void)
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
	return true;
}

bool app_wifi_wait(void)
{
	EventBits_t evt_res = xEventGroupWaitBits(g_app_evt, APP_EVT_WIFI_CONNECTED, false, true, (30 * 1000) / portTICK_PERIOD_MS );
	return (evt_res & APP_EVT_WIFI_CONNECTED);
}

bool app_mqtt_init(void)
{
	return true;
}

void task_control(void *arg)
{
	int main_loops = 0;
	int ntp_reset_counter = 0;
	bool ntp_set = false;

	while (true)
	{
		main_loops++;

		// 0. Init WiFi and set time
		app_wifi_init();
		if (!app_wifi_wait()) {
			// Error!
			ESP_LOGE(TAG, "Wifi connection FAILED!");
			g_clock_warning = true;
			vTaskDelay( (30 * 1000) / portTICK_PERIOD_MS);
			continue; // Try to reconnect
		}

		// 1. Start NTP client

		// Once per X hours - force drop time to ensure time is set. And wait for it
		if ( !ntp_set || (main_loops-ntp_reset_counter) >= APP_FREQ_TIMEDROP ) {
			// Force erase and reset time
			ntp_set = false;
			int ntp_retry = 0;
			const int ntp_retry_count = 10;
			struct timeval tm_zero = {0,0};
			settimeofday(&tm_zero, NULL);
			app_sntp_init();
			if (!app_sntp_wait()) { // Force wait
				ESP_LOGE(TAG, "NTP connection FAILED!");
				g_clock_warning = true;
				continue;
			} else {
				// Okay
				ntp_set = true;
			}
		} else {
			app_sntp_init(); // Just run NTP in background
		}

		// Wait for WLAN up - to allow NTP to set time
		vTaskDelay( (APP_TIME_WLAN * 1000) / portTICK_PERIOD_MS);

		// Flush scan results to MQTT
		if (!app_mqtt_init())
		{
			ESP_LOGE(TAG, "MQTT connection FAILED!");
			g_clock_warning = true;
		}

		// Stop wifi and go to scan mode
		ESP_ERROR_CHECK( esp_wifi_stop() );
		ESP_ERROR_CHECK( esp_ble_gap_start_scanning(10) );
		vTaskDelay( (APP_TIME_SCAN * 1000) / portTICK_PERIOD_MS);

		// All ok?
		g_clock_warning = false;
	}
}
