#include "jwaoo_pwm.h"

static uint8_t jwaoo_pwm_enable_mask;

static void jwaoo_pwm_set_level_base(uint8_t pwm, uint8_t level);
static void jwaoo_led_set_level(uint8_t pwm, uint8_t level);
static void jwaoo_moto_set_level(uint8_t pwm, uint8_t level);

struct jwaoo_pwm_device jwaoo_pwm_devices[] = {
	[JWAOO_PWM_MOTO] = {
		.port = MOTO_GPIO_PORT,
		.pin = MOTO_GPIO_PIN,
		.set_level = jwaoo_moto_set_level,
	},
	[JWAOO_PWM_BT_LED] = {
		.port = BT_LED_GPIO_PORT,
		.pin = BT_LED_GPIO_PIN,
		.set_level = jwaoo_led_set_level,
	},
	[JWAOO_PWM_BATT_LED] = {
		.port = BATT_LED_GPIO_PORT,
		.pin = BATT_LED_GPIO_PIN,
		.set_level = jwaoo_led_set_level,
	},
};

static void jwaoo_pwm_set_enable(uint8_t pwm, bool enable)
{
	if (enable) {
		if (jwaoo_pwm_enable_mask == 0) {
			set_tmr_enable(CLK_PER_REG_TMR_ENABLED);
			set_tmr_div(CLK_PER_REG_TMR_DIV_1);
			timer2_init(HW_CAN_NOT_PAUSE_PWM_2_3_4, PWM_2_3_4_SW_PAUSE_ENABLED, PWM_LEVEL_MAX);
		}

		jwaoo_pwm_enable_mask |= 1 << pwm;
	} else {
		jwaoo_pwm_enable_mask &= ~(1 << pwm);
		if (jwaoo_pwm_enable_mask == 0) {
			timer2_stop();
			set_tmr_enable(CLK_PER_REG_TMR_DISABLED);
		}
	}
}

static void jwaoo_pwm_set_level_base(uint8_t pwm, uint8_t level)
{
	struct jwaoo_pwm_device *device = jwaoo_pwm_get_device(pwm);

	if (device->active_low) {
		level = PWM_LEVEL_MAX - level;
	}

	if (level <= 0) {
		jwaoo_pwm_set_enable(pwm, false);
		GPIO_ConfigurePin(device->port, device->pin, OUTPUT, PID_GPIO, false);
	} else if (level >= PWM_LEVEL_MAX) {
		jwaoo_pwm_set_enable(pwm, false);
		GPIO_ConfigurePin(device->port, device->pin, OUTPUT, PID_GPIO, true);
	} else {
		jwaoo_pwm_set_enable(pwm, true);

		timer2_set_sw_pause(PWM_2_3_4_SW_PAUSE_ENABLED);
		SetWord16(PWM2_DUTY_CYCLE + (pwm * 2), level);
		timer2_set_sw_pause(PWM_2_3_4_SW_PAUSE_DISABLED);
		GPIO_ConfigurePin(device->port, device->pin, OUTPUT, (GPIO_FUNCTION) (PID_PWM2 + pwm), device->active_low);
	}
}

static void jwaoo_led_set_level(uint8_t pwm, uint8_t level)
{
	struct jwaoo_pwm_device *device = jwaoo_pwm_get_device(pwm);

	jwaoo_pwm_set_level_base(pwm, level);
	device->level = level;
}

static void jwaoo_moto_set_level(uint8_t pwm, uint8_t level)
{
	struct jwaoo_pwm_device *device = jwaoo_pwm_get_device(pwm);

	if (level > 0 && device->level == 0) {
		jwaoo_pwm_set_level(pwm, JWAOO_MOTO_BOOST_LEVEL);
		ke_timer_set(JWAOO_MOTO_BOOST, TASK_JWAOO_APP, JWAOO_MOTO_BOOST_TIME);
	} else {
		jwaoo_pwm_set_level(pwm, level);
	}

	device->level = level;
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

bool jwaoo_pwm_blink_walk(uint8_t pwm)
{
	uint8_t level;
	bool complete;
	struct jwaoo_pwm_device *device = jwaoo_pwm_get_device(pwm);

	complete = (device->blink_delay == 0);
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
			jwaoo_pwm_timer_set(pwm, device->blink_delay);
		}
	}

	device->set_level(pwm, level);

	return complete;
}

void jwaoo_pwm_blink_set(uint8_t pwm, uint8_t min, uint8_t max, uint8_t step, uint8_t delay, uint8_t count)
{
	struct jwaoo_pwm_device *device = jwaoo_pwm_get_device(pwm);

	device->set_level(pwm, min);

	if (min < max && step > 0) {
		device->blink_add = true;
		device->blink_min = min;
		device->blink_max = max;
		device->blink_step = step;
		device->blink_delay = delay;
		device->blink_count = count;

		jwaoo_pwm_timer_set(pwm, delay);
	} else {
		jwaoo_pwm_timer_clear(pwm);

		device->blink_min = device->blink_max = min;
		device->blink_delay = 0;
	}
}

void jwaoo_pwm_blink_sawtooth(uint8_t pwm, uint8_t min, uint8_t max, uint8_t step, uint32_t cycle, uint8_t count)
{
	uint8_t delay = cycle * step / (max - min) / 20;
	if (delay < 1) {
		delay = 1;
	}

	jwaoo_pwm_blink_set(pwm, min, max, step, delay, count);
}

void jwaoo_pwm_blink_square(uint8_t pwm, uint8_t min, uint8_t max, uint32_t cycle, uint8_t count)
{
	uint8_t delay = cycle / 20;
	if (delay < 1) {
		delay = 1;
	}

	jwaoo_pwm_blink_set(pwm, min, max, max - min, delay, count);
}
