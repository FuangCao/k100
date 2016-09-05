#include "jwaoo_moto.h"

uint8_t jwaoo_moto_speed_to_level(uint8_t speed)
{
	if (speed < 1) {
		return 0;
	}

	return (speed - 1) * MOTO_LEVEL_STEP + MOTO_LEVEL_MIN;
}

uint8_t jwaoo_moto_level_to_speed(uint8_t level)
{
	if (level < MOTO_LEVEL_MIN) {
		return 0;
	}

	return (level - MOTO_LEVEL_MIN) / MOTO_LEVEL_STEP + 1;
}

void jwaoo_moto_set_speed(uint8_t speed)
{
	uint8_t level = jwaoo_moto_speed_to_level(speed);

	jwaoo_pwm_blink_set_level(JWAOO_PWM_MOTO, level);
}

uint8_t jwaoo_moto_get_speed(void)
{
	uint8_t level = jwaoo_pwm_get_level(JWAOO_PWM_MOTO);

	return jwaoo_moto_level_to_speed(level);
}

bool jwaoo_moto_set_mode(uint8_t mode, uint8_t speed)
{
	switch (mode) {
	case JWAOO_MOTO_MODE_MANUL:
		jwaoo_moto_set_speed(speed);
		break;

	case JWAOO_MOTO_MODE_LINE:
		jwaoo_moto_set_speed(10);
		break;

	case JWAOO_MOTO_MODE_SAWTOOTH:
		jwaoo_moto_blink_sawtooth(4000);
		break;

	case JWAOO_MOTO_MODE_SAWTOOTH_FAST:
		jwaoo_moto_blink_sawtooth(2000);
		break;

	case JWAOO_MOTO_MODE_SQUARE:
		jwaoo_moto_blink_square(4000);
		break;

	case JWAOO_MOTO_MODE_SQUARE_FAST:
		jwaoo_moto_blink_square(2000);
		break;

	case JWAOO_MOTO_MODE_RAND:
		jwaoo_app_timer_set(JWAOO_MOTO_RAND, 1);
		break;

	default:
		return false;
	}

	jwaoo_app_env.moto_mode = mode;

	return true;
}

uint8_t jwaoo_moto_speed_add(void)
{
	uint8_t speed = jwaoo_moto_get_speed();

	if (speed < MOTO_SPEED_MAX) {
		speed++;
	}

	jwaoo_moto_set_mode(JWAOO_MOTO_MODE_MANUL, speed);

	return speed;
}

uint8_t jwaoo_moto_speed_sub(void)
{
	uint8_t speed = jwaoo_moto_get_speed();

	if (speed > 0) {
		speed--;
	}

	jwaoo_moto_set_mode(JWAOO_MOTO_MODE_MANUL, speed);

	return speed;
}

uint8_t jwaoo_moto_mode_add(void)
{
	uint8_t mode = jwaoo_app_env.moto_mode + 1;
	if (mode > JWAOO_MOTO_MODE_COUNT) {
		mode = 1;
	}

	jwaoo_moto_set_mode(mode, 0);

	return mode;
}

void jwaoo_moto_rand_fire(void)
{
}
