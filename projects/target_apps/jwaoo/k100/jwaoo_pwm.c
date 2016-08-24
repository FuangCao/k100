#include "jwaoo_pwm.h"

bool jwaoo_pwm_set_level(struct jwaoo_pwm_device *device, uint8_t level)
{
	if (device->active_low) {
		level = PWM_LEVEL_MAX - level;
	}

	if (level < 1) {
		GPIO_ConfigurePin(device->port, device->pin, OUTPUT, PID_GPIO, false);
	} else if (level > PWM_LEVEL_MAX) {
		GPIO_ConfigurePin(device->port, device->pin, OUTPUT, PID_GPIO, true);
	} else {
		uint8_t pwm = device->pwm;

		timer2_set_sw_pause(PWM_2_3_4_SW_PAUSE_ENABLED);
		SetWord16(PWM2_DUTY_CYCLE + (pwm * 2), level);
		timer2_set_sw_pause(PWM_2_3_4_SW_PAUSE_DISABLED);
		GPIO_ConfigurePin(device->port, device->pin, OUTPUT, (GPIO_FUNCTION) (PID_PWM2 + pwm), device->active_low);
	}

	return true;
}

static bool jwaoo_pwm_set_blink_direction(struct jwaoo_pwm_device *device, bool add)
{
	device->blink_add = add;

	if (device->blink_count == 0) {
		return false;
	}

	if (--device->blink_count) {
		return false;
	}

	device->blink_delay = 0;

	return true;
 }

bool jwaoo_pwm_blink_walk(struct jwaoo_pwm_device *device)
{
	uint8_t level;
	bool complete = (device->blink_delay == 0);

	if (complete) {
		level = device->blink_min;
	} else {
		if (device->blink_add) {
			level = device->level + device->blink_step;
			if (level > device->blink_max) {
				level = device->blink_max - device->blink_step;
				complete = jwaoo_pwm_set_blink_direction(device, false);
			}
		} else {
			level = device->level - device->blink_step;
			if (level < device->blink_min || level > device->blink_max) {
				level = device->blink_min + device->blink_step;
				complete = jwaoo_pwm_set_blink_direction(device, true);
			}
		}

		if (complete) {
			level = device->blink_min;
		} else {
			ke_timer_set(device->blink_timer, TASK_APP, device->blink_delay);
		}
	}

	device->set_level(device, level);

	return complete;
}

void jwaoo_pwm_blink_set(struct jwaoo_pwm_device *device, uint8_t min, uint8_t max, uint8_t step, uint8_t delay, uint8_t count)
{
	device->set_level(device, min);

	if (min < max && step > 0) {
		device->blink_add = true;
		device->blink_min = min;
		device->blink_max = max;
		device->blink_step = step;
		device->blink_delay = delay;
		device->blink_count = count;

		ke_timer_set(device->blink_timer, TASK_APP, 1);
	} else {
		ke_timer_clear(device->blink_timer, TASK_APP);

		device->blink_min = device->blink_max = min;
		device->blink_delay = 0;
	}
}

void jwaoo_pwm_blink_sawtooth(struct jwaoo_pwm_device *device, uint8_t min, uint8_t max, uint8_t step, uint32_t cycle, uint8_t count)
{
	uint8_t delay = cycle * step / (max - min) / 20;
	if (delay < 1) {
		delay = 1;
	}

	jwaoo_pwm_blink_set(device, min, max, step, delay, count);
}

void jwaoo_pwm_blink_square(struct jwaoo_pwm_device *device, uint8_t min, uint8_t max, uint32_t cycle, uint8_t count)
{
	uint8_t delay = cycle / 20;
	if (delay < 1) {
		delay = 1;
	}

	jwaoo_pwm_blink_set(device, min, max, max - min, delay, count);
}

void jwaoo_pwm_init(void)
{
	set_tmr_enable(CLK_PER_REG_TMR_ENABLED);
	set_tmr_div(CLK_PER_REG_TMR_DIV_1);
	timer2_init(HW_CAN_NOT_PAUSE_PWM_2_3_4, PWM_2_3_4_SW_PAUSE_ENABLED, PWM_LEVEL_MAX);
}
