#pragma once

#include "jwaoo_hw.h"
#include "jwaoo_app.h"
#include "pwm.h"

#define JWAOO_PWM_LEVEL_MAX			1000
#define JWAOO_BT_LED_MAX_CURRENT	2000

#define JWAOO_PWM_TIMER(pwm) \
	(JWAOO_PWM1_BLINK_TIMER + (pwm))

#ifdef CFG_JWAOO_PWM_BT_LED
#define BT_LED_OPEN \
	jwaoo_pwm_blink_set_level(JWAOO_PWM_BT_LED, jwaoo_pwm_get_level_by_voltage(jwaoo_app_env.battery_voltage))

#define BT_LED_CLOSE \
	jwaoo_pwm_blink_close(JWAOO_PWM_BT_LED)

#define BT_LED_STATE \
	(jwaoo_pwm_get_level(JWAOO_PWM_BT_LED) > 0)
#else
#define BT_LED_OPEN				GPIO_SetActive(BT_LED_GPIO_PORT, BT_LED_GPIO_PIN)
#define BT_LED_CLOSE			GPIO_SetInactive(BT_LED_GPIO_PORT, BT_LED_GPIO_PIN)
#define BT_LED_STATE			GPIO_GetPinStatus(BT_LED_GPIO_PORT, BT_LED_GPIO_PIN)
#endif

enum {
#ifdef CFG_JWAOO_PWM_MOTO
	JWAOO_PWM_MOTO,
#endif
#ifdef CFG_JWAOO_PWM_BT_LED
	JWAOO_PWM_BT_LED,
#endif
#ifdef CFG_JWAOO_PWM_BATT_LED
	JWAOO_PWM_BATT_LED,
#endif
};

struct jwaoo_pwm_device {
	GPIO_PORT port;
	GPIO_PIN pin;
	bool active_low;

	bool blink_add;
	uint8_t blink_delay;
	uint8_t blink_count;
	uint16_t blink_step;
	uint16_t blink_min;
	uint16_t blink_max;
	uint16_t level;

	void (*set_level)(struct jwaoo_pwm_device *device, uint8_t pwm, uint16_t level);
	void (*on_complete)(struct jwaoo_pwm_device *device);
};

extern struct jwaoo_pwm_device jwaoo_pwms[];

struct jwaoo_pwm_device *jwaoo_pwm_get_device(uint8_t pwm);
void jwaoo_pwm_set_level(uint8_t pwm, uint16_t level);
void jwaoo_pwm_sync(uint8_t pwm);
void jwaoo_pwm_blink_walk(uint8_t pwm);
void jwaoo_pwm_blink_set(uint8_t pwm, uint16_t min, uint16_t max, uint16_t step, uint8_t delay, uint8_t count, bool reload);
void jwaoo_pwm_blink_sawtooth(uint8_t pwm, uint16_t min, uint16_t max, uint16_t step, uint32_t cycle, uint8_t count, bool reload);
void jwaoo_pwm_blink_square(uint8_t pwm, uint16_t min, uint16_t max, uint32_t cycle, uint8_t count, bool reload);
uint16_t jwaoo_pwm_get_level_by_voltage(uint32_t voltage);

static inline struct jwaoo_pwm_device *jwaoo_pwm_get_device(uint8_t pwm)
{
	return jwaoo_pwms + pwm;
}

static inline uint16_t jwaoo_pwm_get_level(uint8_t pwm)
{
	return jwaoo_pwm_get_device(pwm)->level;
}

static inline void jwaoo_pwm_set_complete(uint8_t pwm)
{
	struct jwaoo_pwm_device *device = jwaoo_pwm_get_device(pwm);

	if (device->on_complete) {
		device->on_complete(device);
	}
}

static inline void jwaoo_pwm_timer_set(uint8_t pwm, uint32_t delay)
{
	jwaoo_app_timer_set(JWAOO_PWM_TIMER(pwm), delay);
}

static inline void jwaoo_pwm_timer_clear(uint8_t pwm)
{
	jwaoo_app_timer_clear(JWAOO_PWM_TIMER(pwm));
}

static inline void jwaoo_pwm_blink_set_level(uint8_t pwm, uint16_t level)
{
	jwaoo_pwm_blink_set(pwm, level, level, 0, 0, 0, true);
}

static inline void jwaoo_pwm_blink_open(uint8_t pwm)
{
	jwaoo_pwm_blink_set_level(pwm, JWAOO_PWM_LEVEL_MAX);
}

static inline void jwaoo_pwm_blink_close(uint8_t pwm)
{
	jwaoo_pwm_blink_set_level(pwm, 0);
}

static inline void jwaoo_pwm_blink_sawtooth_full(uint8_t pwm, uint16_t step, uint32_t cycle, uint8_t count)
{
	jwaoo_pwm_blink_sawtooth(pwm, 0, JWAOO_PWM_LEVEL_MAX, step, cycle, count, true);
}

static inline void jwaoo_pwm_blink_square_full(uint8_t pwm, uint32_t cycle, uint8_t count)
{
	jwaoo_pwm_blink_square(pwm, 0, JWAOO_PWM_LEVEL_MAX, cycle, count, true);
}
