#include "jwaoo_moto.h"
#include "co_math.h"

#ifdef MOTO_GPIO_PORT
uint16_t jwaoo_moto_speed_table[JWAOO_MOTO_SPEED_MAX + 1] = {
	// 0, 20, 25, 35, 50, 70, 95, 125, 160, 200, 245, 295, 350, 410, 475, 545, 620, 700, JWAOO_PWM_LEVEL_MAX
	0, 20, 25, 30, 40, 55, 72, 93, 118, 146, 177, 212, 251, 293, 338, 387, 440, 496, 555
};

static inline void jwaoo_moto_set_speed(uint8_t speed)
{
	jwaoo_pwm_blink_set_level(JWAOO_PWM_MOTO, speed);
}

static inline void jwaoo_moto_blink_sawtooth(uint32_t cycle, int speed, bool reload)
{
	jwaoo_pwm_blink_sawtooth(JWAOO_PWM_MOTO, 1, speed, 1, cycle, 0, reload);
}

static inline void jwaoo_moto_blink_square(uint32_t cycle, int speed, bool reload)
{
	jwaoo_pwm_blink_square(JWAOO_PWM_MOTO, 0, speed, cycle, 0, reload);
}

uint16_t jwaoo_moto_speed_to_level(uint16_t speed)
{
	return jwaoo_moto_speed_table[speed];
}

bool jwaoo_moto_speed_add(int value)
{
	int speed;

	if (jwaoo_app_env.moto_mode == JWAOO_MOTO_MODE_RAND && jwaoo_app_settings.moto_rand_max > 0) {
		return (value > 0);
	}

	speed = value + jwaoo_app_env.moto_speed;
	if (value < 0) {
		if (speed < jwaoo_app_settings.moto_speed_min) {
			return false;
		}
	} else if (speed > JWAOO_MOTO_SPEED_MAX) {
		return false;
	} else if (speed < jwaoo_app_settings.moto_speed_min) {
		speed = jwaoo_app_settings.moto_speed_min;
	}

	switch (jwaoo_app_env.moto_mode) {
	case JWAOO_MOTO_MODE_IDLE:
		jwaoo_app_env.moto_mode = JWAOO_MOTO_MODE_LINE;
	case JWAOO_MOTO_MODE_LINE:
		jwaoo_moto_set_speed(speed);
		break;

	case JWAOO_MOTO_MODE_SAWTOOTH:
		jwaoo_moto_blink_sawtooth(JWAOO_MOTO_SAWTOOTH_LONG, speed, false);
		break;

	case JWAOO_MOTO_MODE_SAWTOOTH_FAST:
		jwaoo_moto_blink_sawtooth(JWAOO_MOTO_SAWTOOTH_SHORT, speed, false);
		break;

	case JWAOO_MOTO_MODE_SQUARE:
		jwaoo_moto_blink_square(JWAOO_MOTO_SQUARE_LONG, speed, false);
		break;

	case JWAOO_MOTO_MODE_SQUARE_FAST:
		jwaoo_moto_blink_square(JWAOO_MOTO_SQUARE_SHORT, speed, false);
		break;
	}

	jwaoo_app_env.moto_speed = speed;

	if (jwaoo_app_env.moto_report) {
		SEND_EMPTY_MESSAGE(JWAOO_TOY_MOTO_REPORT_STATE, TASK_JWAOO_TOY);
	}

	return true;
}

void jwaoo_moto_set_mode(uint8_t mode)
{
	if (mode < JWAOO_MOTO_MODE_FIRST || mode > JWAOO_MOTO_MODE_LAST) {
		jwaoo_app_env.moto_mode = JWAOO_MOTO_MODE_IDLE;
		jwaoo_app_env.moto_speed = 0;
		jwaoo_pwm_blink_close(JWAOO_PWM_MOTO);
		jwaoo_app_suspend_counter_start();
	} else {
		jwaoo_app_env.moto_mode = mode;

		if (jwaoo_app_env.moto_speed < jwaoo_app_settings.moto_speed_min) {
			jwaoo_app_env.moto_speed = jwaoo_app_settings.moto_speed_min;
		}

		switch (mode) {
		case JWAOO_MOTO_MODE_LINE:
			jwaoo_moto_set_speed(jwaoo_app_env.moto_speed);
			break;

		case JWAOO_MOTO_MODE_SAWTOOTH:
			jwaoo_moto_blink_sawtooth(JWAOO_MOTO_SAWTOOTH_LONG, jwaoo_app_env.moto_speed, true);
			break;

		case JWAOO_MOTO_MODE_SAWTOOTH_FAST:
			jwaoo_moto_blink_sawtooth(JWAOO_MOTO_SAWTOOTH_SHORT, jwaoo_app_env.moto_speed, true);
			break;

		case JWAOO_MOTO_MODE_SQUARE:
			jwaoo_moto_blink_square(JWAOO_MOTO_SQUARE_LONG, jwaoo_app_env.moto_speed, true);
			break;

		case JWAOO_MOTO_MODE_SQUARE_FAST:
			jwaoo_moto_blink_square(JWAOO_MOTO_SQUARE_SHORT, jwaoo_app_env.moto_speed, true);
			break;

		case JWAOO_MOTO_MODE_RAND:
			jwaoo_app_timer_set(JWAOO_MOTO_RAND_TIMER, 1);
			break;
		}
	}

	if (jwaoo_app_env.moto_report) {
		SEND_EMPTY_MESSAGE(JWAOO_TOY_MOTO_REPORT_STATE, TASK_JWAOO_TOY);
	}
}

uint8_t jwaoo_moto_mode_add(void)
{
	uint8_t mode = jwaoo_app_env.moto_mode + 1;

	if (mode > JWAOO_MOTO_MODE_LAST) {
		mode = JWAOO_MOTO_MODE_FIRST;
	}

	jwaoo_moto_set_mode(mode);

	return mode;
}

void jwaoo_moto_rand_timer_fire(void)
{
	if (jwaoo_app_env.moto_mode == JWAOO_MOTO_MODE_RAND) {
		uint8_t max;

		if (jwaoo_app_settings.moto_rand_max > 0) {
			max = jwaoo_app_settings.moto_rand_max;
		} else if (jwaoo_app_env.moto_speed > 0) {
			max = jwaoo_app_env.moto_speed;
		} else {
			return;
		}

		jwaoo_moto_set_speed((rand() % max) + 1);
		jwaoo_app_timer_set(JWAOO_MOTO_RAND_TIMER, jwaoo_app_settings.moto_rand_delay);
	}
}
#endif
