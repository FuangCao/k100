#pragma once

#include "jwaoo_hw.h"
#include "pwm.h"

#define PWM_LEVEL_MAX		100

struct jwaoo_pwm_device
{
	uint8_t pwm;
	GPIO_PORT port;
	GPIO_PIN pin;
	bool active_low;
	uint16_t blink_timer;

	uint8_t level;
	bool blink_add;
	uint8_t blink_step;
	uint8_t blink_min;
	uint8_t blink_max;
	uint8_t blink_delay;
	uint8_t blink_count;

	void (*set_level)(struct jwaoo_pwm_device *device, uint8_t level);
};

void jwaoo_pwm_init(void);
