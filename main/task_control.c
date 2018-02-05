#include "task_control.h"
#include "app_scan.h"
#include "app_mqtt.h"

static bool app_sntp_init(void);
static bool app_sntp_wait(void);
static bool app_wifi_init(void);
static bool app_wifi_wait(void);

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

void task_control(void *arg)
{
	ESP_LOGI(TAG, "task_control Init");

	int main_loops = 0;
	int ntp_reset_counter = 0;
	bool ntp_set = false;

	app_wifi_init();
	app_wifi_wait();
	app_sntp_init();
	app_mqtt_init();
	app_scan_init();

	while (true) {
		if (!g_mqtt_connected) {
			app_mqtt_start();
		}

		vTaskDelay( (3 * 1000) / portTICK_PERIOD_MS);
	}
}
