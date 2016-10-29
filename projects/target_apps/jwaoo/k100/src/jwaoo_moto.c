#include "jwaoo_moto.h"
#include "co_math.h"

static uint16_t jwaoo_moto_speed_table[] = {
	0, 20, 25, 35, 50, 70, 95, 125, 160, 200, 245, 295, 350, 410, 475, 545, 620, 700, JWAOO_PWM_LEVEL_MAX
};

static void jwaoo_moto_set_speed(uint8_t speed)
{
	if (speed == 0) {
		jwaoo_app_suspend_counter_start();
	}

	jwaoo_pwm_blink_set_level(JWAOO_PWM_MOTO, speed);
}

static inline void jwaoo_moto_blink_sawtooth(uint32_t cycle, int speed)
{
	jwaoo_pwm_blink_sawtooth(JWAOO_PWM_MOTO, JWAOO_MOTO_SPEED_MIN, speed, 1, cycle, 0);
}

static bool jwaoo_moto_set_speed_sawtooth(uint32_t cycle, int speed)
{
	if (speed > JWAOO_MOTO_SPEED_MIN) {
		if (speed > JWAOO_MOTO_SPEED_MAX) {
			speed = JWAOO_MOTO_SPEED_MAX;
		}

		if (speed != jwaoo_app_env.moto_speed_sawtooth) {
			jwaoo_app_env.moto_speed_sawtooth = speed;
			jwaoo_moto_blink_sawtooth(cycle, speed);
		}

		return true;
	}

	jwaoo_app_env.moto_speed_sawtooth = JWAOO_MOTO_SPEED_MIN;

	return false;
}

static void jwaoo_moto_set_speed_sawtooth_first(uint32_t cycle, uint8_t speed)
{
	if (speed > JWAOO_MOTO_SPEED_MIN) {
		jwaoo_app_env.moto_speed_sawtooth = speed;
	} else if (jwaoo_app_env.moto_speed_sawtooth <= JWAOO_MOTO_SPEED_MIN) {
		jwaoo_app_env.moto_speed_sawtooth = JWAOO_MOTO_SPEED_MIN + 1;
	}

	jwaoo_moto_blink_sawtooth(cycle, jwaoo_app_env.moto_speed_sawtooth);
}

static inline void jwaoo_moto_blink_square(uint32_t cycle, int speed)
{
	jwaoo_pwm_blink_square(JWAOO_PWM_MOTO, 0, speed, cycle, 0);
}

static bool jwaoo_moto_set_speed_square(uint32_t cycle, int speed)
{
	if (speed > 0) {
		if (speed > JWAOO_MOTO_SPEED_MAX) {
			speed = JWAOO_MOTO_SPEED_MAX;
		}

		if (speed != jwaoo_app_env.moto_speed_square) {
			jwaoo_app_env.moto_speed_square = speed;
			jwaoo_moto_blink_square(cycle, speed);
		}

		return true;
	}

	jwaoo_app_env.moto_speed_square = 0;

	return false;
}

static void jwaoo_moto_set_speed_square_first(uint32_t cycle, uint8_t speed)
{
	if (speed > 0) {
		jwaoo_app_env.moto_speed_square = speed;
	} else if (jwaoo_app_env.moto_speed_square <= 0) {
		jwaoo_app_env.moto_speed_square = 1;
	}

	jwaoo_moto_blink_square(cycle, jwaoo_app_env.moto_speed_square);
}

static bool jwaoo_moto_set_speed_line(int speed)
{
	if (speed > 0) {
		if (speed > JWAOO_MOTO_SPEED_MAX) {
			speed = JWAOO_MOTO_SPEED_MAX;
		}

		if (speed != jwaoo_app_env.moto_speed_line) {
			jwaoo_app_env.moto_speed_line = speed;
			jwaoo_moto_set_speed(speed);
		}

		return true;
	}

	jwaoo_app_env.moto_speed_line = 0;

	return false;
}

static void jwaoo_moto_set_speed_line_first(uint8_t speed)
{
	if (speed > 0) {
		jwaoo_app_env.moto_speed_line = speed;
	} else if (jwaoo_app_env.moto_speed_line <= 0) {
		jwaoo_app_env.moto_speed_line = 1;
	}

	jwaoo_moto_set_speed(jwaoo_app_env.moto_speed_line);
}

static void jwaoo_moto_set_rand_enable(bool enable)
{
	if (enable) {
		jwaoo_app_env.moto_rand = 1;
		jwaoo_app_timer_set(JWAOO_MOTO_RAND_TIMER, 1);
	} else {
		jwaoo_app_timer_clear(JWAOO_MOTO_RAND_TIMER);
		jwaoo_app_env.moto_rand = 0;
	}
}

uint16_t jwaoo_moto_speed_to_level(uint16_t speed)
{
	return jwaoo_moto_speed_table[speed];
}

bool jwaoo_moto_speed_add(int value)
{
	bool enable;

	switch (jwaoo_app_env.moto_mode) {
	case JWAOO_MOTO_MODE_LINE:
		enable = jwaoo_moto_set_speed_line(value + jwaoo_app_env.moto_speed_line);
		break;

	case JWAOO_MOTO_MODE_SAWTOOTH:
		enable = jwaoo_moto_set_speed_sawtooth(JWAOO_MOTO_DELAY_LONG, value + jwaoo_app_env.moto_speed_sawtooth);
		break;

	case JWAOO_MOTO_MODE_SAWTOOTH_FAST:
		enable = jwaoo_moto_set_speed_sawtooth(JWAOO_MOTO_DELAY_SHORT, value + jwaoo_app_env.moto_speed_sawtooth);
		break;

	case JWAOO_MOTO_MODE_SQUARE:
		enable = jwaoo_moto_set_speed_square(JWAOO_MOTO_DELAY_LONG, value + jwaoo_app_env.moto_speed_square);
		break;

	case JWAOO_MOTO_MODE_SQUARE_FAST:
		enable = jwaoo_moto_set_speed_square(JWAOO_MOTO_DELAY_SHORT, value + jwaoo_app_env.moto_speed_square);
		break;

	case JWAOO_MOTO_MODE_RAND:
		enable = (value > 0);
		jwaoo_moto_set_rand_enable(enable);
		break;

	default:
		enable = false;
	}

	if (!enable) {
		jwaoo_moto_blink_close();
	}

	if (jwaoo_app_env.moto_report) {
		SEND_EMPTY_MESSAGE(JWAOO_TOY_MOTO_REPORT_STATE, TASK_JWAOO_TOY);
	}

	return enable;
}

bool jwaoo_moto_set_mode(uint8_t mode, uint8_t speed)
{
	switch (mode) {
	case JWAOO_MOTO_MODE_IDLE:
		jwaoo_moto_blink_close();
		break;

	case JWAOO_MOTO_MODE_LINE:
		jwaoo_moto_set_speed_line_first(speed);
		break;

	case JWAOO_MOTO_MODE_SAWTOOTH:
		jwaoo_moto_set_speed_sawtooth_first(JWAOO_MOTO_DELAY_LONG, speed);
		break;

	case JWAOO_MOTO_MODE_SAWTOOTH_FAST:
		jwaoo_moto_set_speed_sawtooth_first(JWAOO_MOTO_DELAY_SHORT, speed);
		break;

	case JWAOO_MOTO_MODE_SQUARE:
		jwaoo_moto_set_speed_square_first(JWAOO_MOTO_DELAY_LONG, speed);
		break;

	case JWAOO_MOTO_MODE_SQUARE_FAST:
		jwaoo_moto_set_speed_square_first(JWAOO_MOTO_DELAY_SHORT, speed);
		break;

	case JWAOO_MOTO_MODE_RAND:
		jwaoo_moto_set_rand_enable(true);
		break;

	default:
		return false;
	}

	jwaoo_app_env.moto_mode = mode;

	if (jwaoo_app_env.moto_report) {
		SEND_EMPTY_MESSAGE(JWAOO_TOY_MOTO_REPORT_STATE, TASK_JWAOO_TOY);
	}

	return true;
}

uint8_t jwaoo_moto_mode_add(void)
{
	uint8_t mode = jwaoo_app_env.moto_mode + 1;

	if (mode > JWAOO_MOTO_MODE_LAST) {
		mode = 1;
	}

	jwaoo_moto_set_mode(mode, 0);

	return mode;
}

void jwaoo_moto_rand_timer_fire(void)
{
	if (jwaoo_app_env.moto_mode == JWAOO_MOTO_MODE_RAND && jwaoo_app_env.moto_rand > 0) {
		uint8_t count;
		uint8_t speed = jwaoo_moto_get_speed();

		for (count = 0; speed == jwaoo_app_env.moto_rand && count < 100; count++) {
			jwaoo_app_env.moto_rand = rand() % JWAOO_MOTO_SPEED_MAX + 1;
		}

		if (speed < jwaoo_app_env.moto_rand) {
			speed++;
		} else {
			speed--;
		}

		jwaoo_moto_set_speed(speed);
		jwaoo_app_timer_set(JWAOO_MOTO_RAND_TIMER, 5);
	}
}
