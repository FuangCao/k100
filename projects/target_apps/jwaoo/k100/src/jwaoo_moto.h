#pragma once

#include "jwaoo_pwm.h"

#define MOTO_LEVEL_MIN			32
#define MOTO_SPEED_MAX			18
#define MOTO_LEVEL_STEP			((PWM_LEVEL_MAX - MOTO_LEVEL_MIN) / (MOTO_SPEED_MAX - 1))

enum
{
	JWAOO_MOTO_MODE_MANUL = 0,
	JWAOO_MOTO_MODE_LINE,
	JWAOO_MOTO_MODE_SAWTOOTH,
	JWAOO_MOTO_MODE_SAWTOOTH_FAST,
	JWAOO_MOTO_MODE_SQUARE,
	JWAOO_MOTO_MODE_SQUARE_FAST,
	JWAOO_MOTO_MODE_RAND,
	JWAOO_MOTO_MODE_COUNT = JWAOO_MOTO_MODE_RAND,
};

uint8_t jwaoo_moto_speed_to_level(uint8_t speed);
uint8_t jwaoo_moto_level_to_speed(uint8_t level);
void jwaoo_moto_set_speed(uint8_t speed);
uint8_t jwaoo_moto_get_speed(void);

bool jwaoo_moto_set_mode(uint8_t mode, uint8_t speed);
uint8_t jwaoo_moto_speed_add(void);
uint8_t jwaoo_moto_speed_sub(void);
uint8_t jwaoo_moto_mode_add(void);
void jwaoo_moto_rand_fire(void);

static inline struct jwaoo_pwm_device *jwaoo_moto_get_device()
{
	return jwaoo_pwm_get_device(JWAOO_PWM_MOTO);
}

static inline void jwaoo_moto_blink_open(void)
{
	jwaoo_pwm_blink_open(JWAOO_PWM_MOTO);
}

static inline void jwaoo_moto_blink_close(void)
{
	jwaoo_pwm_blink_close(JWAOO_PWM_MOTO);
}

static inline void jwaoo_moto_blink_sawtooth(uint32_t cycle)
{
	jwaoo_pwm_blink_sawtooth(JWAOO_PWM_MOTO, MOTO_LEVEL_MIN, PWM_LEVEL_MAX, MOTO_LEVEL_STEP, cycle, 0);
}

static inline void jwaoo_moto_blink_square(uint32_t cycle)
{
	jwaoo_pwm_blink_square(JWAOO_PWM_MOTO, 0, PWM_LEVEL_MAX, cycle, 0);
}

