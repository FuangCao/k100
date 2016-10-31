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
	jwaoo_pwm_blink_sawtooth(JWAOO_PWM_MOTO, 1, speed, 1, cycle, 0);
}

void jwaoo_moto_blink_close(void)
{
	jwaoo_app_env.moto_mode = JWAOO_MOTO_MODE_IDLE;
	jwaoo_pwm_blink_close(JWAOO_PWM_MOTO);
}

static inline void jwaoo_moto_blink_square(uint32_t cycle, int speed)
{
	jwaoo_pwm_blink_square(JWAOO_PWM_MOTO, 0, speed, cycle, 0);
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

	if (jwaoo_app_env.moto_mode == JWAOO_MOTO_MODE_RAND) {
		enable = (value > 0);
	} else {
		int speed = value + jwaoo_app_env.moto_speed;

		if (value < 0 && speed < JWAOO_MOTO_SPEED_MIN) {
			enable = false;
		} else {
			if (speed > JWAOO_MOTO_SPEED_MAX) {
				speed = JWAOO_MOTO_SPEED_MAX;
			} else if (speed < JWAOO_MOTO_SPEED_MIN) {
				speed = JWAOO_MOTO_SPEED_MIN;
			}

			if (speed == jwaoo_app_env.moto_speed) {
				return true;
			}

			jwaoo_app_env.moto_speed = speed;
			enable = true;

			switch (jwaoo_app_env.moto_mode) {
			case JWAOO_MOTO_MODE_IDLE:
				jwaoo_app_env.moto_mode = JWAOO_MOTO_MODE_LINE;
			case JWAOO_MOTO_MODE_LINE:
				jwaoo_moto_set_speed(speed);
				break;

			case JWAOO_MOTO_MODE_SAWTOOTH:
				jwaoo_moto_blink_sawtooth(JWAOO_MOTO_SAWTOOTH_LONG, speed);
				break;

			case JWAOO_MOTO_MODE_SAWTOOTH_FAST:
				jwaoo_moto_blink_sawtooth(JWAOO_MOTO_SAWTOOTH_SHORT, speed);
				break;

			case JWAOO_MOTO_MODE_SQUARE:
				jwaoo_moto_blink_square(JWAOO_MOTO_SQUARE_LONG, speed);
				break;

			case JWAOO_MOTO_MODE_SQUARE_FAST:
				jwaoo_moto_blink_square(JWAOO_MOTO_SQUARE_SHORT, speed);
				break;
			}
		}
	}

	if (!enable) {
		jwaoo_app_env.moto_speed = 0;
		jwaoo_moto_blink_close();
	}

	if (jwaoo_app_env.moto_report) {
		SEND_EMPTY_MESSAGE(JWAOO_TOY_MOTO_REPORT_STATE, TASK_JWAOO_TOY);
	}

	return enable;
}

bool jwaoo_moto_set_mode(uint8_t mode)
{
	if (jwaoo_app_env.moto_speed < JWAOO_MOTO_SPEED_MIN) {
		jwaoo_app_env.moto_speed = JWAOO_MOTO_SPEED_MIN;
	}

	switch (mode) {
	case JWAOO_MOTO_MODE_IDLE:
		jwaoo_pwm_blink_close(JWAOO_PWM_MOTO);
		break;

	case JWAOO_MOTO_MODE_LINE:
		jwaoo_moto_set_speed(jwaoo_app_env.moto_speed);
		break;

	case JWAOO_MOTO_MODE_SAWTOOTH:
		jwaoo_moto_blink_sawtooth(JWAOO_MOTO_SAWTOOTH_LONG, jwaoo_app_env.moto_speed);
		break;

	case JWAOO_MOTO_MODE_SAWTOOTH_FAST:
		jwaoo_moto_blink_sawtooth(JWAOO_MOTO_SAWTOOTH_SHORT, jwaoo_app_env.moto_speed);
		break;

	case JWAOO_MOTO_MODE_SQUARE:
		jwaoo_moto_blink_square(JWAOO_MOTO_SQUARE_LONG, jwaoo_app_env.moto_speed);
		break;

	case JWAOO_MOTO_MODE_SQUARE_FAST:
		jwaoo_moto_blink_square(JWAOO_MOTO_SQUARE_SHORT, jwaoo_app_env.moto_speed);
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
		mode = JWAOO_MOTO_MODE_FIRST;
	}

	jwaoo_moto_set_mode(mode);

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
