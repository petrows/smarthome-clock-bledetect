#include "app_mqtt.h"

char client_id[24];

static void mqtt_status_cb(esp_mqtt_status_t status)
{
	ESP_LOGI(TAG, "MQTT status %d", status);

	switch (status) {
	case ESP_MQTT_STATUS_CONNECTED:
		xEventGroupSetBits(g_app_evt, APP_EVT_MQTT_CONNECTED);
		gpio_set_level(GPIO_NUM_2, 1);
		esp_mqtt_publish("pws/clock-ble/boot", (uint8_t*)client_id, strlen(client_id), 0, false);
		break;
	case ESP_MQTT_STATUS_DISCONNECTED:
		gpio_set_level(GPIO_NUM_2, 1);
		xEventGroupClearBits(g_app_evt, APP_EVT_MQTT_CONNECTED);
		break;
	}
}

static void mqtt_message_cb(const char *topic, uint8_t *payload, size_t len)
{
	ESP_LOGI(TAG, "MQTT msg:\t%s:%s (%d)\n", topic, payload, (int) len);
}

bool app_mqtt_init(void)
{
	gpio_pad_select_gpio(GPIO_NUM_2);
	gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);

	esp_mqtt_init(mqtt_status_cb, mqtt_message_cb, 256, 2000);
	return true;
}

bool app_mqtt_start(void)
{
	uint8_t mac[6];
	bzero(client_id, 24);
	esp_wifi_get_mac(ESP_IF_WIFI_STA, mac);
	snprintf(client_id, 24, "esp32-%02x%02x%02x%02x", (int)mac[2], (int)mac[3], (int)mac[4], (int)mac[5]);
	ESP_LOGI(TAG, "MQTT client id %s", client_id);
	esp_mqtt_start(CONFIG_MQTT_HOST, CONFIG_MQTT_PORT, (const char*)client_id, CONFIG_MQTT_USER, CONFIG_MQTT_PASS);
	return true;
}

bool app_mqtt_stop(void)
{
	esp_mqtt_stop();
	return true;
}

void app_mqtt_send_message(const char * topic, const char * data, int len)
{
	esp_mqtt_publish(topic, (uint8_t*)data, len, 0, false);
}

/*
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
 }*/
