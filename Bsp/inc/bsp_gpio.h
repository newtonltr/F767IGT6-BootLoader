#ifndef BSP_GPIO_H
#define BSP_GPIO_H

#include "main.h"
#include <stdint.h>

#define WHEEL_FRONT_PIN DI5_Pin
#define WHEEL_FRONT_PORT DI5_GPIO_Port

#define WHEEL_FRONT_TRIGGER 0
#define WHEEL_FRONT_RELEASE 1

struct gpio_input_filter_t {
	uint16_t pin;
	GPIO_TypeDef *port;
	uint8_t io_state_real;
	uint8_t io_state_real_last;
	uint8_t io_state_filter;

	uint16_t filter_cnt;
	uint16_t filter_cnt_max;

	// 用于判断长按
	uint16_t trigger_cnt;
	uint16_t trigger_cnt_max;
	uint8_t trigger_long_press_flag;

};

extern struct gpio_input_filter_t wheel_front_filter;

void gpio_input_filter(struct gpio_input_filter_t *filter, uint8_t trigger_level);

#endif
