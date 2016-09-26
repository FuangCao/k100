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
			timer2_init(HW_CAN_NOT_PAUSE_PWM_2_3_4, PWM_2_3_4_SW_PAUSE_ENABLED, PWM_LEVEL_MAX);
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

static void jwaoo_pwm_device_set_level_handler(struct jwaoo_pwm_device *device, uint8_t pwm, uint8_t level)
{
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

static void jwaoo_moto_device_set_level_handler(struct jwaoo_pwm_device *device, uint8_t pwm, uint8_t level)
{
	if (jwaoo_app_env.moto_boost_busy) {
		return;
	}

	if (level > 0 && device->level == 0) {
		jwaoo_app_env.moto_boost_busy = true;
		jwaoo_pwm_device_set_level_handler(device, pwm, MOTO_BOOST_LEVEL);
		jwaoo_app_timer_set(JWAOO_MOTO_BOOST, MOTO_BOOST_TIME);
	} else {
		jwaoo_pwm_device_set_level_handler(device, pwm, level);
	}
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

static void jwaoo_pwm_device_set_level(struct jwaoo_pwm_device *device, uint8_t pwm, uint8_t level)
{
	device->set_level(device, pwm, level);
	device->level = level;
}

void jwaoo_pwm_set_level(uint8_t pwm, uint8_t level)
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

	jwaoo_pwm_device_set_level(device, pwm, level);

	if (complete && device->on_complete) {
		device->on_complete(device);
	}
}

void jwaoo_pwm_blink_set(uint8_t pwm, uint8_t min, uint8_t max, uint8_t step, uint8_t delay, uint8_t count)
{
	struct jwaoo_pwm_device *device = jwaoo_pwm_get_device(pwm);

	jwaoo_pwm_device_set_level(device, pwm, min);

	if (min < max && step > 0) {
		jwaoo_pwm_timer_set(pwm, delay);

		device->blink_add = true;
		device->blink_min = min;
		device->blink_max = max;
		device->blink_step = step;
		device->blink_delay = delay;
		device->blink_count = count;
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
	[JWAOO_PWM_BT_LED] = {
		.port = BT_LED_GPIO_PORT,
		.pin = BT_LED_GPIO_PIN,
		.set_level = jwaoo_pwm_device_set_level_handler,
	},
	[JWAOO_PWM_BATT_LED] = {
		.port = BATT_LED_GPIO_PORT,
		.pin = BATT_LED_GPIO_PIN,
		.set_level = jwaoo_pwm_device_set_level_handler,
		.on_complete = jwaoo_battery_led_complete,
	},
};
