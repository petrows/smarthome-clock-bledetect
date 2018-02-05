#include "app_mqtt.h"

mqtt_client * g_mqtt = NULL;
bool g_mqtt_connected = false;

static void app_mqtt_cb_connected(mqtt_client *client, mqtt_event_data_t *event_data)
{
	ESP_LOGI(TAG, "MQTT connected");
	g_mqtt_connected = true;
}

static void app_mqtt_cb_disconnected(mqtt_client *client, mqtt_event_data_t *event_data)
{
	ESP_LOGI(TAG, "MQTT disconnected");
	g_mqtt_connected = false;
	g_mqtt = NULL;
}

static void app_mqtt_cb_subscribe(mqtt_client *client, mqtt_event_data_t *event_data)
{

}

static void app_mqtt_cb_publish(mqtt_client *client, mqtt_event_data_t *event_data)
{

}
static void app_mqtt_cb_data(mqtt_client *client, mqtt_event_data_t *event_data)
{

}

struct mqtt_settings app_mqtt_settings = {
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

bool app_mqtt_init(void)
{
	if (NULL != g_mqtt) { return false; }

	// Init MQTT config
	g_mqtt = mqtt_start(&app_mqtt_settings);

	return true;
}

bool app_mqtt_wait(void)
{
	for (int ntp_retry=0; ntp_retry<10; ++ntp_retry)
	{
		if (g_mqtt_connected) {
			return true;
		}
		vTaskDelay(2000 / portTICK_PERIOD_MS);
	}
	return false;
}

bool app_mqtt_stop(void)
{
	g_mqtt_connected = false;

	if (NULL != g_mqtt) {
		mqtt_stop(g_mqtt);
		g_mqtt = NULL;
	}
	return true;
}

bool app_mqtt_clear(void)
{
	return true;
}
