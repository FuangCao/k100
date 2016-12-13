#pragma once

#include "jwaoo_pwm.h"

#define JWAOO_MOTO_SPEED_MIN			6
#define JWAOO_MOTO_SPEED_MAX			18
#define JWAOO_MOTO_BOOST_LEVEL			JWAOO_PWM_LEVEL_MAX
#define JWAOO_MOTO_BOOST_TIME			2

#define JWAOO_MOTO_SAWTOOTH_SHORT		800
#define JWAOO_MOTO_SAWTOOTH_LONG		1600
#define JWAOO_MOTO_SQUARE_SHORT			600
#define JWAOO_MOTO_SQUARE_LONG			1200

enum
{
	JWAOO_MOTO_MODE_IDLE = 0,
	JWAOO_MOTO_MODE_LINE,
	JWAOO_MOTO_MODE_FIRST = JWAOO_MOTO_MODE_LINE,
	JWAOO_MOTO_MODE_SAWTOOTH,
	JWAOO_MOTO_MODE_SAWTOOTH_FAST,
	JWAOO_MOTO_MODE_SQUARE,
	JWAOO_MOTO_MODE_SQUARE_FAST,
	JWAOO_MOTO_MODE_RAND,
	JWAOO_MOTO_MODE_LAST = JWAOO_MOTO_MODE_RAND,
	JWAOO_MOTO_MODE_COUNT,
};

extern uint16_t jwaoo_moto_speed_table[];

uint16_t jwaoo_moto_speed_to_level(uint16_t speed);
uint8_t jwaoo_moto_get_speed(void);

void jwaoo_moto_set_mode(uint8_t mode);
uint8_t jwaoo_moto_mode_add(void);
bool jwaoo_moto_speed_add(int value);
void jwaoo_moto_rand_timer_fire(void);

#ifdef CFG_JWAOO_PWM_MOTO
static inline struct jwaoo_pwm_device *jwaoo_moto_get_device()
{
	return jwaoo_pwm_get_device(JWAOO_PWM_MOTO);
}

static inline uint8_t jwaoo_moto_get_speed(void)
{
	return jwaoo_pwm_get_level(JWAOO_PWM_MOTO);
}

static inline void jwaoo_moto_blink_close(void)
{
	jwaoo_moto_set_mode(JWAOO_MOTO_MODE_IDLE);
}
#endif
