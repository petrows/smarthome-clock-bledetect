#include "task_display.h"

#include <tm1637.h>

#define LED_ONBOARD GPIO_NUM_2
#define LED_CLOCK GPIO_NUM_25

#define BRIGHTNESS_MIN 1
#define BRIGHTNESS_MAX 7
#define BRIGHTNESS_DELTA (BRIGHTNESS_MAX - BRIGHTNESS_MIN)

#define BRIGHTNESS_M_BGN 6
#define BRIGHTNESS_M_END 8

#define BRIGHTNESS_E_BGN 18
#define BRIGHTNESS_E_END 22

void task_display(void *arg)
{
	tm1637_lcd_t * lcd = tm1637_init(GPIO_NUM_26, GPIO_NUM_27);

	char strftime_buf[64];

	time_t now = 0;
	struct tm timeinfo = { 0 };
	bool dot_ptr = true;
	uint8_t min_prev = 0;
	uint8_t brightness = BRIGHTNESS_MAX;
	uint8_t brightness_prev = 0;
	
	int x = 0;
	
	gpio_set_direction(LED_CLOCK, GPIO_MODE_OUTPUT);
	gpio_set_direction(LED_ONBOARD, GPIO_MODE_OUTPUT);
	gpio_set_level(LED_CLOCK, 0);
	gpio_set_level(LED_ONBOARD, 0);

	while (true) {
		x++;
		if (g_clock_warning) {
			dot_ptr = true;
		} else {
			dot_ptr = !dot_ptr;
		}

		time(&now);
		localtime_r(&now, &timeinfo);

		if (timeinfo.tm_year < (2016 - 1900)) {
			// Time is not set!
			tm1637_set_segment_raw(lcd, 0, 0x40);
			tm1637_set_segment_raw(lcd, 1, 0x40);
			tm1637_set_segment_raw(lcd, 2, 0x40);
			tm1637_set_segment_raw(lcd, 3, 0x40);
		} else {
			uint8_t clock_h = timeinfo.tm_hour;
			uint8_t clock_m = timeinfo.tm_min;

			if (clock_h < 10) {
				tm1637_set_segment_number(lcd, 0, 0xFF, dot_ptr);
				tm1637_set_segment_number(lcd, 1, clock_h, dot_ptr);
			} else {
				tm1637_set_segment_number(lcd, 0, clock_h / 10, dot_ptr);
				tm1637_set_segment_number(lcd, 1, clock_h % 10, dot_ptr);
			}

			if (clock_m < 10) {
				tm1637_set_segment_number(lcd, 2, 0, dot_ptr);
				tm1637_set_segment_number(lcd, 3, clock_m, dot_ptr);
			} else {
				tm1637_set_segment_number(lcd, 2, clock_m / 10, dot_ptr);
				tm1637_set_segment_number(lcd, 3, clock_m % 10, dot_ptr);
			}

			// Set brightness
			if (clock_h >= BRIGHTNESS_M_BGN && clock_h <= BRIGHTNESS_M_END) {
				float min_from_start = ((clock_h - BRIGHTNESS_M_BGN) * 60) + clock_m;
				float min_period = (BRIGHTNESS_M_END - BRIGHTNESS_M_BGN) * 60;
				brightness = ((float)BRIGHTNESS_DELTA * (min_from_start/min_period));
				brightness += BRIGHTNESS_MIN;
			} else if (clock_h >= BRIGHTNESS_E_BGN && clock_h <= BRIGHTNESS_E_END) {
				float min_from_start = ((clock_h - BRIGHTNESS_E_BGN) * 60) + clock_m;
				float min_period = (BRIGHTNESS_E_END - BRIGHTNESS_E_BGN) * 60;
				brightness = BRIGHTNESS_MAX - ((float)BRIGHTNESS_DELTA * (min_from_start/min_period));
			}

			tm1637_set_brightness(lcd, brightness);
		}

		if (brightness_prev != brightness) {
			brightness_prev = brightness;
			ESP_LOGI(TAG, "Brightness: %d", (int)brightness);
		}

		if (min_prev != timeinfo.tm_min) {
			min_prev = timeinfo.tm_min;
			// Print current time (for debug)
			strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
			ESP_LOGI(TAG, "The current date/time is: %s", strftime_buf);
		}
		
		EventBits_t app_status = xEventGroupGetBits(g_app_evt);
		bool wifi_ok = app_status & APP_EVT_WIFI_CONNECTED;
		bool mqtt_ok = app_status & APP_EVT_MQTT_CONNECTED;
		gpio_set_level(LED_ONBOARD, wifi_ok && mqtt_ok);

		for (; g_led_signal != 0; --g_led_signal) {
			gpio_set_level(LED_CLOCK, true);
			vTaskDelay(50 / portTICK_PERIOD_MS);
			gpio_set_level(LED_CLOCK, false);
			vTaskDelay(50 / portTICK_PERIOD_MS);
		}
		
		vTaskDelay(500 / portTICK_PERIOD_MS);
	}
}
