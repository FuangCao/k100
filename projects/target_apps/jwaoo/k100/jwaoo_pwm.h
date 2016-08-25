#pragma once

#include "jwaoo_hw.h"
#include "jwaoo_app.h"
#include "pwm.h"

#define JWAOO_PWM_LEVEL_MAX		100
#define JWAOO_MOTO_BOOST_LEVEL	JWAOO_PWM_LEVEL_MAX
#define JWAOO_MOTO_BOOST_TIME	5

#define JWAOO_PWM_TIMER(pwm) \
	(JWAOO_PWM1_BLINK_TIMER + (pwm))

enum
{
	JWAOO_PWM_MOTO,
	JWAOO_PWM_BT_LED,
	JWAOO_PWM_BATT_LED,
};

struct jwaoo_pwm_device
{
	GPIO_PORT port;
	GPIO_PIN pin;
	bool active_low;

	uint8_t level;
	bool blink_add;
	uint8_t blink_step;
	uint8_t blink_min;
	uint8_t blink_max;
	uint8_t blink_delay;
	uint8_t blink_count;

	void (*set_level)(struct jwaoo_pwm_device *device, uint8_t pwm, uint8_t level);
	void (*on_complete)(struct jwaoo_pwm_device *device);
};

extern struct jwaoo_pwm_device jwaoo_pwm_devices[];

struct jwaoo_pwm_device *jwaoo_pwm_get_device(uint8_t pwm);
void jwaoo_pwm_blink_walk(uint8_t pwm);
void jwaoo_pwm_blink_set(uint8_t pwm, uint8_t min, uint8_t max, uint8_t step, uint8_t delay, uint8_t count);
void jwaoo_pwm_blink_sawtooth(uint8_t pwm, uint8_t min, uint8_t max, uint8_t step, uint32_t cycle, uint8_t count);
void jwaoo_pwm_blink_square(uint8_t pwm, uint8_t min, uint8_t max, uint32_t cycle, uint8_t count);

static inline struct jwaoo_pwm_device *jwaoo_pwm_get_device(uint8_t pwm)
{
	return jwaoo_pwm_devices + pwm;
}

static inline void jwaoo_pwm_timer_set(uint8_t pwm, uint32_t delay)
{
	ke_timer_set(JWAOO_PWM_TIMER(pwm), TASK_JWAOO_APP, delay);
}

static inline void jwaoo_pwm_timer_clear(uint8_t pwm)
{
	ke_timer_clear(JWAOO_PWM_TIMER(pwm), TASK_JWAOO_APP);
}

static inline void jwaoo_pwm_set_level(uint8_t pwm, uint8_t level)
{
	jwaoo_pwm_blink_set(pwm, level, level, 0, 0, 0);
}

static inline void jwaoo_pwm_open(uint8_t pwm)
{
	jwaoo_pwm_set_level(pwm, JWAOO_PWM_LEVEL_MAX);
}

static inline void jwaoo_pwm_close(uint8_t pwm)
{
	jwaoo_pwm_set_level(pwm, 0);
}

static inline void jwaoo_pwm_blink_sawtooth_full(uint8_t pwm, uint8_t step, uint32_t cycle, uint8_t count)
{
	jwaoo_pwm_blink_sawtooth(pwm, 0, JWAOO_PWM_LEVEL_MAX, step, cycle, count);
}

static inline void jwaoo_pwm_blink_square_full(uint8_t pwm, uint32_t cycle, uint8_t count)
{
	jwaoo_pwm_blink_square(pwm, 0, JWAOO_PWM_LEVEL_MAX, cycle, count);
}

