#include "adc.h"
#include "jwaoo_app.h"
#include "jwaoo_pwm.h"
#include "jwaoo_battery.h"
#include "jwaoo_toy_task.h"

#define DEBUG_AVG_VOLTAGE		0

static const struct jwaoo_battery_voltage_map jwaoo_battery_voltage_table[] = {
	{ 2048, 1800 },
	{ 2106, 1850 },
	{ 2163, 1900 },
	{ 2222, 1950 },
	{ 2278, 2000 },
	{ 2337, 2050 },
	{ 2393, 2100 },
	{ 2450, 2150 },
	{ 2509, 2200 },
	{ 2566, 2250 },
	{ 2623, 2300 },
	{ 2682, 2350 },
	{ 2740, 2400 },
	{ 2761, 2420 },
	{ 2784, 2440 },
	{ 2808, 2460 },
	{ 2829, 2480 },
	{ 2852, 2500 },
	{ 2877, 2520 },
	{ 2899, 2540 },
	{ 2922, 2560 },
	{ 2944, 2580 },
	{ 2970, 2600 },
	{ 2992, 2620 },
	{ 3014, 2640 },
	{ 3037, 2660 },
	{ 3061, 2680 },
	{ 3083, 2700 },
	{ 3106, 2720 },
	{ 3129, 2740 },
	{ 3152, 2760 },
	{ 3176, 2780 },
	{ 3198, 2800 },
	{ 3221, 2820 },
	{ 3244, 2840 },
	{ 3291, 2880 },
	{ 3312, 2900 },
	{ 3335, 2920 },
	{ 3359, 2940 },
	{ 3382, 2960 },
	{ 3405, 2980 },
	{ 3427, 3000 },
	{ 3484, 3050 },
	{ 3544, 3100 },
	{ 3599, 3150 },
	{ 3657, 3200 },
};

static struct jwaoo_led_current_map jwaoo_led_current_table[] = {
	{ 2700, 1892 },
	{ 2750, 2686 },
	{ 2800, 3675 },
	{ 2850, 4845 },
	{ 2900, 6200 },
	{ 2950, 7725 },
	{ 3000, 9460 },
	{ 3050, 11350 },
	{ 3100, 13440 },
	{ 3150, 15630 },
	{ 3200, 17938 },
};

#ifdef CHG_DET_GPIO_PORT
static void jwaoo_charge_isr(struct jwaoo_irq_desc *desc, bool status)
{
	jwaoo_battery_poll_start();
}

struct jwaoo_irq_desc jwaoo_charge = {
	.port = CHG_DET_GPIO_PORT,
	.pin = CHG_DET_GPIO_PIN,
	.handler = jwaoo_charge_isr,
};
#endif

void jwaoo_battery_set_enable(bool enable)
{
#ifdef CHG_DET_GPIO_PORT
	if (enable) {
		jwaoo_hw_irq_enable(CHG_DET_GPIO_IRQ, &jwaoo_charge, CHG_DET_ACTIVE_LOW);
	} else {
		jwaoo_hw_irq_disable(CHG_DET_GPIO_IRQ);
	}
#endif
}

void jwaoo_battery_led_blink(void)
{
	if (jwaoo_app_env.battery_led_locked < 2) {
		jwaoo_app_env.battery_led_locked = 1;
#ifdef CFG_JWAOO_PWM_BATT_LED
		jwaoo_pwm_blink_square_full(JWAOO_PWM_BATT_LED, jwaoo_key_settings.led_blink_delay, 1);
#elif defined(CFG_JWAOO_PWM_BT_LED)
		jwaoo_pwm_blink_square(JWAOO_PWM_BT_LED, 0, jwaoo_pwm_get_level_by_voltage(jwaoo_app_env.battery_voltage), jwaoo_key_settings.led_blink_delay, 1, true);
#endif
	}
}

void jwaoo_battery_led_set_enable(bool enable)
{
	if (enable) {
		if (jwaoo_app_env.battery_led_locked < 2) {
			jwaoo_app_env.battery_led_locked = 1;
#ifdef CFG_JWAOO_PWM_BATT_LED
			jwaoo_pwm_blink_open(JWAOO_PWM_BATT_LED);
#elif defined(CFG_JWAOO_PWM_BT_LED)
			jwaoo_pwm_blink_set_level(JWAOO_PWM_BT_LED, jwaoo_pwm_get_level_by_voltage(jwaoo_app_env.battery_voltage));
#endif
		}
	} else {
#ifdef CFG_JWAOO_PWM_BATT_LED
		jwaoo_pwm_blink_close(JWAOO_PWM_BATT_LED);
#elif defined(CFG_JWAOO_PWM_BT_LED)
		jwaoo_pwm_blink_close(JWAOO_PWM_BT_LED);
#endif
		jwaoo_battery_led_release(1, false);
	}
}

void jwaoo_battery_led_release(uint8_t level, bool force)
{
	if (jwaoo_app_env.battery_led_locked <= level || force) {
		jwaoo_battery_led_update_state(true);
	}
}

void jwaoo_battery_led_update_state(bool force)
{
	if (force) {
		jwaoo_app_env.battery_led_locked = 0;
	} else if (jwaoo_app_env.battery_led_locked) {
		return;
	}

#ifdef CFG_JWAOO_PWM_BATT_LED
	switch (jwaoo_app_env.battery_state) {
	case JWAOO_TOY_BATTERY_LOW:
		jwaoo_pwm_blink_square_full(JWAOO_PWM_BATT_LED, 500, 0);
		break;

	case JWAOO_TOY_BATTERY_FULL:
		jwaoo_pwm_blink_open(JWAOO_PWM_BATT_LED);
		break;

	case JWAOO_TOY_BATTERY_CHARGING:
		jwaoo_pwm_blink_sawtooth_full(JWAOO_PWM_BATT_LED, JWAOO_PWM_LEVEL_MAX / 50, 4000, 0);
		break;

	default:
		jwaoo_pwm_blink_close(JWAOO_PWM_BATT_LED);
	}
#elif defined(CFG_JWAOO_PWM_BT_LED)
	jwaoo_pwm_blink_close(JWAOO_PWM_BT_LED);
	jwaoo_app_timer_set(JWAOO_BT_LED_BLINK, jwaoo_app_settings.bt_led_close_time);
#endif
}

void jwaoo_battery_set_state(uint8_t state)
{
	if (jwaoo_app_env.battery_state != state) {
		jwaoo_app_env.battery_state = state;
#ifdef CFG_JWAOO_PWM_BATT_LED
		jwaoo_battery_led_update_state(false);
#endif
	}
}

uint16_t jwaoo_battery_voltage_calibration(uint32_t voltage)
{
	static const struct jwaoo_battery_voltage_map *map0 = jwaoo_battery_voltage_table;
	static const struct jwaoo_battery_voltage_map *map1 = jwaoo_battery_voltage_table + 1;
	static const struct jwaoo_battery_voltage_map *map_tail = jwaoo_battery_voltage_table + NELEM(jwaoo_battery_voltage_table) - 1;

	if (voltage < map0->raw_value) {
		while (1) {
			if (map0 > jwaoo_battery_voltage_table) {
				map1 = map0--;

				if (voltage >= map0->raw_value) {
					break;
				}
			} else {
				return voltage * map0->real_value / map0->raw_value;
			}
		}
	} else if (voltage > map1->raw_value) {
		while (map1 < map_tail) {
			map0 = map1++;

			if (voltage <= map1->raw_value) {
				break;
			}
		}
	}

	return (voltage - map0->raw_value) * (map1->real_value - map0->real_value) / (map1->raw_value - map0->raw_value) + map0->real_value;
}

#if DEBUG_AVG_VOLTAGE
static volatile uint32_t avg_count;
static volatile uint32_t avg_voltage;
#endif

void jwaoo_battery_poll(bool optimize)
{
	int i;
	uint8_t state;
	uint8_t level;
	uint32_t voltage;
#ifdef CHG_ONLINE
	bool charge_online = CHG_ONLINE;
#endif

#if DEBUG_AVG_VOLTAGE
	jwaoo_app_timer_set(JWAOO_BATT_POLL_TIMER, 5);
#else
	jwaoo_app_timer_set(JWAOO_BATT_POLL_TIMER, JWAOO_BATT_POLL_DELAY);
#endif

	adc_calibrate();

	SetWord16(GP_ADC_CTRL_REG, GP_ADC_LDO_EN | GP_ADC_SE | GP_ADC_EN | ADC_CHANNEL_VBAT3V << 6);
	SetWord16(GP_ADC_CTRL2_REG, GP_ADC_DELAY_EN | GP_ADC_I20U | GP_ADC_ATTN3X);

	for (i = 0, voltage = 0; i < 4; i++) {
		SetBits16(GP_ADC_CTRL_REG, GP_ADC_SIGN, i & 1);
		voltage += adc_get_sample();
	}

	adc_disable();

	if (jwaoo_app_env.battery_skip < 3) {
		jwaoo_app_env.battery_skip++;
		return;
	}

#if DEBUG_AVG_VOLTAGE
	if (avg_count++ > 0) {
		avg_voltage = (avg_voltage * 7 + voltage) >> 3;

		if (avg_count > 200) {
			avg_count = 0;
		}
	} else {
		avg_voltage = voltage;
	}
#else
	println("raw voltage = %d", voltage);

	voltage = jwaoo_battery_voltage_calibration(voltage);

	if (optimize) {
		if (voltage < JWAOO_BATT_VOLTAGE_VALID_MIN || voltage > JWAOO_BATT_VOLTAGE_VALID_MAX) {
			voltage = JWAOO_BATT_VOLTAGE_MAX;
		} else {
#ifdef CHG_ONLINE
			if (charge_online && voltage < 4226 && jwaoo_app_env.battery_state != JWAOO_TOY_BATTERY_FULL) {
				uint8_t percent;

				if (voltage < 4100) {
					percent = 96;
				} else if (voltage < 4200) {
					percent = 97;
				} else if (voltage < 4210) {
					percent = 98;
				} else {
					percent = 99;
				}

				voltage = voltage * percent / 100;
				if (voltage < jwaoo_app_env.battery_voltage) {
					voltage = jwaoo_app_env.battery_voltage;
				}
			}

			println("fix voltage = %d", voltage);
#endif

			jwaoo_app_env.battery_voltages[jwaoo_app_env.battery_voltage_head] = voltage;
			jwaoo_app_env.battery_voltage_head = (jwaoo_app_env.battery_voltage_head + 1) % JWAOO_VOLTAGE_ARRAY_SIZE;

			if (jwaoo_app_env.battery_voltage_count < JWAOO_VOLTAGE_ARRAY_SIZE) {
				jwaoo_app_env.battery_voltage_count++;
			}

			for (i = jwaoo_app_env.battery_voltage_count - 1, voltage = 0; i >= 0; i--) {
				voltage += jwaoo_app_env.battery_voltages[i];
			}

			voltage /= jwaoo_app_env.battery_voltage_count;

			println("avg voltage = %d", voltage);
		}
	}

	jwaoo_app_env.battery_voltage = voltage;

	if (voltage <= JWAOO_BATT_VOLTAGE_MIN) {
		level = 0;
	} else if (voltage >= JWAOO_BATT_VOLTAGE_MAX) {
		level = 100;
	} else {
		level = (voltage - JWAOO_BATT_VOLTAGE_MIN) * 100 / (JWAOO_BATT_VOLTAGE_MAX - JWAOO_BATT_VOLTAGE_MIN);
	}

#ifdef CHG_ONLINE
	if (charge_online) {
		if (BATT_CHARGING) {
			jwaoo_app_env.battery_full = 0;
			state = JWAOO_TOY_BATTERY_CHARGING;
		} else if (jwaoo_app_env.battery_full < 10) {
			jwaoo_app_env.battery_full++;
			state = JWAOO_TOY_BATTERY_CHARGING;
		} else {
			state = JWAOO_TOY_BATTERY_FULL;
		}
	} else
#else
	{
#endif
		jwaoo_app_env.battery_full = 0;

		if (level > JWAOO_BATT_LEVEL_LOW) {
			state = JWAOO_TOY_BATTERY_NORMAL;
		} else {
			state = JWAOO_TOY_BATTERY_LOW;

#if 0
			if (voltage < jwaoo_app_settings.shutdown_voltage) {
				jwaoo_app_goto_suspend_mode();
			}
#endif
		}
	}

	jwaoo_app_env.battery_level = level;
	jwaoo_battery_set_state(state);

	if (jwaoo_app_env.battery_report) {
		SEND_EMPTY_MESSAGE(JWAOO_TOY_BATT_REPORT_STATE, TASK_JWAOO_TOY);
	}
#endif
}

uint16_t jwaoo_battery_get_led_max_current(uint32_t voltage)
{
	const struct jwaoo_led_current_map *map = jwaoo_led_current_table;
	const struct jwaoo_led_current_map *map_end = map + NELEM(jwaoo_led_current_table) - 1;

	while (map < map_end && map->voltage < voltage) {
		map++;
	}

	return voltage * map->current / map->voltage;
}
