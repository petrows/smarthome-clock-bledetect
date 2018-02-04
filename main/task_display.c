#include "task_display.h"

#include <tm1637.h>

void task_display(void *arg)
{
	tm1637_lcd_t * lcd = tm1637_init(GPIO_NUM_18, GPIO_NUM_19);

	char strftime_buf[64];

	time_t now = 0;
	struct tm timeinfo = { 0 };
	bool dot_ptr = true;
	uint8_t min_prev = 0;

	while (true) {
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
		}

		if (min_prev != timeinfo.tm_min) {
			min_prev = timeinfo.tm_min;
			// Print current time (for debug)
			strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
			ESP_LOGI(TAG, "The current date/time is: %s", strftime_buf);
		}

		vTaskDelay(500 / portTICK_PERIOD_MS);
	}
}
