#include "task_display.h"

#include <tm1637.h>

void task_display(void *arg)
{
	tm1637_lcd_t * lcd = tm1637_init(GPIO_NUM_18, GPIO_NUM_19);

	time_t now = 0;
	struct tm timeinfo = { 0 };
	bool dot_ptr = true;

	while (true) {
		if (g_clock_warning) {
			dot_ptr = true;
		} else {
			dot_ptr = !dot_ptr;
		}

		time(&now);
		localtime_r(&now, &timeinfo);

		char strftime_buf[64];
		strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
		ESP_LOGI(TAG, "The current date/time in Berlin is: %s", strftime_buf);

		if (timeinfo.tm_year < (2016 - 1900)) {
			// Time is not set!
			tm1637_set_segment_raw(lcd, 0, 0x40);
			tm1637_set_segment_raw(lcd, 1, 0x40);
			tm1637_set_segment_raw(lcd, 2, 0x40);
			tm1637_set_segment_raw(lcd, 3, 0x40);
		} else {
			tm1637_set_number_lead_dot(lcd, (timeinfo.tm_hour * 100) + timeinfo.tm_min, false, dot_ptr ? 0x00 : 0xFF);
		}

		vTaskDelay(500 / portTICK_PERIOD_MS);
	}
}
