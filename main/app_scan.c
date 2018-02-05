#include "app_scan.h"

static esp_ble_scan_params_t app_scan_params =
{
	.scan_type              = BLE_SCAN_TYPE_PASSIVE,
	.own_addr_type          = BLE_ADDR_TYPE_PUBLIC,
	.scan_filter_policy     = BLE_SCAN_FILTER_ALLOW_ALL,
	.scan_interval          = 0x50,
	.scan_window            = 0x30,
};

static void app_scan_gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
	uint8_t *adv_name = NULL;
	uint8_t adv_name_len = 0;

	switch (event) {
	case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT: {
		ESP_LOGI(TAG, "Scan param set complete, start scanning.");
		esp_ble_gap_start_scanning(30);
		break;
	}

	case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
		if (param->scan_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
			ESP_LOGE(TAG, "Scan start failed.");
		} else {
			ESP_LOGI(TAG, "Scan start successfully.");
		}
		break;

	case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
		if (param->scan_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) {
			ESP_LOGE(TAG, "Scan stop failed.");
		} else {
			ESP_LOGI(TAG, "Scan stop successfully.");
		}
		break;

	case ESP_GAP_BLE_SCAN_RESULT_EVT: {
		esp_ble_gap_cb_param_t *scan_result = (esp_ble_gap_cb_param_t *) param;
		switch (scan_result->scan_rst.search_evt) {
		case ESP_GAP_SEARCH_INQ_RES_EVT: {
			if (0x36 == scan_result->scan_rst.bda[5] || 0xd8 == scan_result->scan_rst.bda[5]) {
				ESP_LOGI(TAG, "I feel device: ");
				esp_log_buffer_hex(TAG, scan_result->scan_rst.bda, 6);
				adv_name = esp_ble_resolve_adv_data(scan_result->scan_rst.ble_adv, ESP_BLE_AD_TYPE_NAME_CMPL, &adv_name_len);
				esp_log_buffer_char(TAG, adv_name, adv_name_len);
			}
			break;
		}

		case ESP_GAP_SEARCH_INQ_CMPL_EVT: {
			ESP_LOGI(TAG, "Scan completed, restarting.");
			// signal_scan_complete();
			esp_ble_gap_set_scan_params(&app_scan_params);
			break;
		}

		default:
			break;
		}
		break;
	}
	default:
		break;
	}
}

void app_scan_init() {
	esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();

	esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
	esp_bt_controller_init(&bt_cfg);
	esp_bt_controller_enable(ESP_BT_MODE_BLE);
	esp_bluedroid_init();
	esp_bluedroid_enable();

	esp_ble_gap_register_callback(app_scan_gap_event_handler);
	esp_ble_gap_set_scan_params(&app_scan_params);
}
