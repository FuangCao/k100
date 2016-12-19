#include "jwaoo_pwm.h"
#include "jwaoo_moto.h"
#include "jwaoo_battery.h"

static void jwaoo_pwm_set_enable(uint8_t pwm, bool enable)
{
	static uint8_t enable_mask;

	if (enable) {
		if (enable_mask == 0) {
			set_tmr_enable(CLK_PER_REG_TMR_ENABLED);
			set_tmr_div(CLK_PER_REG_TMR_DIV_1);
			timer2_init(HW_CAN_NOT_PAUSE_PWM_2_3_4, PWM_2_3_4_SW_PAUSE_ENABLED, JWAOO_PWM_LEVEL_MAX);
		}

		enable_mask |= 1 << pwm;
	} else {
		enable_mask &= ~(1 << pwm);
		if (enable_mask == 0) {
			timer2_stop();
			set_tmr_enable(CLK_PER_REG_TMR_DISABLED);
		}
	}
}

static void jwaoo_pwm_device_set_level_handler(struct jwaoo_pwm_device *device, uint8_t pwm, uint16_t level)
{
	if (device->active_low) {
		level = JWAOO_PWM_LEVEL_MAX - level;
	}

	if (level <= 0) {
		jwaoo_pwm_set_enable(pwm, false);
		GPIO_ConfigurePin(device->port, device->pin, OUTPUT, PID_GPIO, false);
	} else if (level >= JWAOO_PWM_LEVEL_MAX) {
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

void jwaoo_pwm_device_set_level_boost(struct jwaoo_pwm_device *device, uint8_t pwm, uint8_t level)
{
	uint8_t boost = jwaoo_app_env.moto_boost_level + JWAOO_MOTO_BOOST_STEP;

	if (level > boost) {
		jwaoo_app_env.moto_boost_level = level = boost;
		jwaoo_app_timer_set(JWAOO_MOTO_BOOST, JWAOO_MOTO_BOOST_DELAY);
	}

	jwaoo_pwm_device_set_level_handler(device, pwm, jwaoo_moto_speed_to_level(level));
}

static void jwaoo_moto_device_set_level_handler(struct jwaoo_pwm_device *device, uint8_t pwm, uint16_t level)
{
	jwaoo_app_env.moto_boost_level = device->level;
	jwaoo_pwm_device_set_level_boost(device, pwm, level);
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

static void jwaoo_pwm_device_set_level(struct jwaoo_pwm_device *device, uint8_t pwm, uint16_t level)
{
	device->set_level(device, pwm, level);
	device->level = level;
}

void jwaoo_pwm_set_level(uint8_t pwm, uint16_t level)
{
	struct jwaoo_pwm_device *device = jwaoo_pwm_get_device(pwm);

	jwaoo_pwm_device_set_level(device, pwm, level);
}

void jwaoo_pwm_sync(uint8_t pwm)
{
	struct jwaoo_pwm_device *device = jwaoo_pwm_get_device(pwm);

	device->set_level(device, pwm, device->level);
}

void jwaoo_pwm_blink_walk(uint8_t pwm)
{
	uint16_t level;
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

	jwaoo_pwm_device_set_level(device, pwm, level);

	if (complete && device->on_complete) {
		device->on_complete(device);
	}
}

void jwaoo_pwm_blink_set(uint8_t pwm, uint16_t min, uint16_t max, uint16_t step, uint8_t delay, uint8_t count, bool reload)
{
	struct jwaoo_pwm_device *device = jwaoo_pwm_get_device(pwm);

	if (min < max && step > 0) {
		if (delay < 1) {
			delay = 1;
		}

		if (count > 0 || device->level > max) {
			device->blink_add = false;
			jwaoo_pwm_device_set_level(device, pwm, max);
		} else if (device->level < min) {
			device->blink_add = true;
			jwaoo_pwm_device_set_level(device, pwm, min);
		}

		if (reload) {
			jwaoo_pwm_timer_set(pwm, delay);
		}

		device->blink_min = min;
		device->blink_max = max;
		device->blink_step = step;
		device->blink_delay = delay;
		device->blink_count = count;
	} else {
		jwaoo_pwm_device_set_level(device, pwm, max);
		jwaoo_pwm_timer_clear(pwm);

		device->blink_min = device->blink_max = min;
		device->blink_delay = 0;
	}
}

void jwaoo_pwm_blink_sawtooth(uint8_t pwm, uint16_t min, uint16_t max, uint16_t step, uint32_t cycle, uint8_t count, bool reload)
{
	uint8_t delay;

	if (max > min) {
		delay = cycle * step / (max - min) / 20;
	} else {
		delay = 0;
	}

	jwaoo_pwm_blink_set(pwm, min, max, step, delay, count, reload);
}

void jwaoo_pwm_blink_square(uint8_t pwm, uint16_t min, uint16_t max, uint32_t cycle, uint8_t count, bool reload)
{
	jwaoo_pwm_blink_set(pwm, min, max, max - min, cycle / 20, count, reload);
}

static void jwaoo_battery_led_complete(struct jwaoo_pwm_device *device)
{
	jwaoo_battery_led_release(1);
}

struct jwaoo_pwm_device jwaoo_pwms[] = {
	[JWAOO_PWM_MOTO] = {
		.port = MOTO_GPIO_PORT,
		.pin = MOTO_GPIO_PIN,
		.set_level = jwaoo_moto_device_set_level_handler,
	},
	[JWAOO_PWM_BATT_LED] = {
		.port = BATT_LED_GPIO_PORT,
		.pin = BATT_LED_GPIO_PIN,
		.set_level = jwaoo_pwm_device_set_level_handler,
		.on_complete = jwaoo_battery_led_complete,
	},
};
