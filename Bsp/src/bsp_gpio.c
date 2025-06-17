#include "bsp_gpio.h"
#include <stdint.h>

struct gpio_input_filter_t wheel_front_filter = {.filter_cnt_max = 1, .trigger_cnt_max = 40};

void gpio_input_filter(struct gpio_input_filter_t *filter, uint8_t trigger_level)
{
	if(filter->io_state_real == filter->io_state_real_last) {
		filter->filter_cnt++;
	} else {
		filter->filter_cnt = 0;
	}

	if(filter->filter_cnt >= filter->filter_cnt_max) {
		filter->io_state_filter = filter->io_state_real;
		filter->filter_cnt = 0;
	}

	if(filter->io_state_real == trigger_level) {
		filter->trigger_cnt++;
	} else {
		filter->trigger_cnt = 0;
		filter->trigger_long_press_flag = 0;
	}

	if(filter->trigger_cnt >= filter->trigger_cnt_max) {
		filter->trigger_cnt = filter->trigger_cnt_max;
		filter->trigger_long_press_flag = 1;
	} else {
		filter->trigger_long_press_flag = 0;
	}

	filter->io_state_real_last = filter->io_state_real;
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	switch(GPIO_Pin)
	{
		case WHEEL_FRONT_PIN:
			wheel_front_filter.io_state_real = HAL_GPIO_ReadPin(WHEEL_FRONT_PORT, WHEEL_FRONT_PIN)? WHEEL_FRONT_RELEASE : WHEEL_FRONT_TRIGGER;	
			break;
		default:
			break;
	}
}

