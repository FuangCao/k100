#include "jwaoo_moto.h"
#include "co_math.h"

static uint16_t jwaoo_moto_speed_table[] = {
	0, 20, 25, 35, 50, 70, 95, 125, 160, 200, 245, 295, 350, 410, 475, 545, 620, 700, JWAOO_PWM_LEVEL_MAX
};

uint16_t jwaoo_moto_speed_to_level(uint16_t speed)
{
	return jwaoo_moto_speed_table[speed];
}

bool jwaoo_moto_set_mode(uint8_t mode, uint8_t speed)
{
	switch (mode) {
	case JWAOO_MOTO_MODE_MANUL:
		jwaoo_moto_set_speed(speed);
		if (speed == 0) {
			jwaoo_app_suspend_counter_start();
		}
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
		jwaoo_app_timer_set(JWAOO_MOTO_RAND_TIMER, 1);
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

uint8_t jwaoo_moto_speed_add(void)
{
	uint8_t speed = jwaoo_moto_get_speed();

	if (speed < JWAOO_MOTO_SPEED_MAX) {
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

void jwaoo_moto_rand_timer_fire(void)
{
	if (jwaoo_app_env.moto_mode == JWAOO_MOTO_MODE_RAND) {
		jwaoo_moto_set_speed(rand() % JWAOO_MOTO_SPEED_MAX + 1);
		jwaoo_app_timer_set(JWAOO_MOTO_RAND_TIMER, 10);
	}
}
