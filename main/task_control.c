#include "task_control.h"

#include <mqtt.h>

static bool app_sntp_init(void);
static bool app_sntp_wait(void);
static bool app_wifi_init(void);
static bool app_wifi_wait(void);
static bool app_mqtt_init(void);
static bool app_mqtt_wait(void);
static bool app_mqtt_stop(void);
static bool app_mqtt_clear(void);

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

mqtt_client * mqtt = NULL;
bool mqtt_connected = false;
static void app_mqtt_cb_connected(mqtt_client *client, mqtt_event_data_t *event_data) {}
static void app_mqtt_cb_disconnected(mqtt_client *client, mqtt_event_data_t *event_data) {}
static void app_mqtt_cb_subscribe(mqtt_client *client, mqtt_event_data_t *event_data) {}
static void app_mqtt_cb_publish(mqtt_client *client, mqtt_event_data_t *event_data) {}
static void app_mqtt_cb_data(mqtt_client *client, mqtt_event_data_t *event_data) {}

bool app_mqtt_init(void)
{
	// Init MQTT config
	struct mqtt_settings settings = {
		.host = CONFIG_MQTT_HOST,
		.port = CONFIG_MQTT_PORT,
		.client_id = "esp32",
		.username = CONFIG_MQTT_USER,
		.password = CONFIG_MQTT_PASS,
		.clean_session = 0,
		.keepalive = 120,
		.lwt_topic = "/lwt",
		.lwt_msg = "offline",
		.lwt_qos = 0,
		.lwt_retain = 0,
		.connected_cb = app_mqtt_cb_connected,
		.disconnected_cb = app_mqtt_cb_disconnected,
		.subscribe_cb = app_mqtt_cb_subscribe,
		.publish_cb = app_mqtt_cb_publish,
		.data_cb = app_mqtt_cb_data
	};

	mqtt_connected = false;
	mqtt = mqtt_start(&settings);

	return true;
}

bool app_mqtt_wait(void)
{
	for (int ntp_retry=0; ntp_retry<10; ++ntp_retry)
	{
		if (mqtt_connected) {
			return true;
		}
		vTaskDelay(2000 / portTICK_PERIOD_MS);
	}
	return false;
}

bool app_mqtt_stop(void)
{
	if (NULL != mqtt) {
		mqtt_stop(mqtt);
		mqtt = NULL;
	}
	return true;
}

bool app_mqtt_clear(void)
{
	return true;
}

void task_control(void *arg)
{
    ESP_LOGI(TAG, "task_control Init");
    ESP_LOGI(TAG, "task_control Free memory: %d bytes", system_get_free_heap_size());
    ESP_LOGI(TAG, "task_control SDK version: %s", system_get_sdk_version());

	int main_loops = 0;
	int ntp_reset_counter = 0;
	bool ntp_set = false;



	while (true)
	{
		main_loops++;

		ESP_LOGI(TAG, "Main loop %d", main_loops);

		// 0. Init WiFi and set time
		app_wifi_init();
		if (!app_wifi_wait()) {
			// Error!
			ESP_LOGE(TAG, "Wifi connection FAILED!");
			g_clock_warning = true;
			esp_wifi_stop(); // Reset it
			vTaskDelay( (30 * 1000) / portTICK_PERIOD_MS);
			continue; // Try to reconnect
		}

		// 1. Start NTP client

		// Once per X hours - force drop time to ensure time is set. And wait for it
		if ( !ntp_set || (main_loops-ntp_reset_counter) >= APP_FREQ_TIMEDROP ) {
			// Force erase and reset time
			ESP_LOGI(TAG, "Force reset NTP time");
			ntp_set = false;
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
				ntp_reset_counter = main_loops;
			}
		} else {
			app_sntp_init(); // Just run NTP in background
		}

		// Flush scan results to MQTT
		if (!app_mqtt_init())
		{
			ESP_LOGE(TAG, "MQTT connection FAILED!");
			g_clock_warning = true;
		}

		// Wait for WLAN up - to allow NTP to set time
		ESP_LOGI(TAG, "WLAN safe wait for %d sec", APP_TIME_WLAN);
		vTaskDelay( (APP_TIME_WLAN * 1000) / portTICK_PERIOD_MS);

		// Stop wifi and go to scan mode
		app_mqtt_clear(); // Clean prev results
		ESP_LOGI(TAG, "Scan BLE for %d sec", APP_TIME_SCAN);
		ESP_ERROR_CHECK( esp_wifi_stop() );
		ESP_ERROR_CHECK( esp_ble_gap_start_scanning(APP_TIME_SCAN) );
		vTaskDelay( (APP_TIME_SCAN * 1000) / portTICK_PERIOD_MS);

		// All ok?
		g_clock_warning = false;
	}
}
