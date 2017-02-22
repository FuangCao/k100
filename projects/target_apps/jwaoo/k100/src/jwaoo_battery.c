#include "adc.h"
#include "jwaoo_app.h"
#include "jwaoo_pwm.h"
#include "jwaoo_battery.h"
#include "jwaoo_toy_task.h"

#define DEBUG_AVG_VOLTAGE		0

static const struct jwaoo_battery_voltage_map jwaoo_battery_voltage_table[] = {
	{ 3102, 2800 },
	{ 3212, 2900 },
	{ 3322, 3000 },
	{ 3441, 3100 },
	{ 3561, 3200 },
	{ 3611, 3250 },
	{ 3665, 3300 },
	{ 3723, 3350 },
	{ 3778, 3400 },
	{ 3833, 3450 },
	{ 3889, 3500 },
	{ 3949, 3550 },
	{ 4008, 3600 },
	{ 4059, 3650 },
	{ 4120, 3700 },
	{ 4175, 3750 },
	{ 4234, 3800 },
	{ 4292, 3850 },
	{ 4349, 3900 },
	{ 4402, 3950 },
	{ 4455, 4000 },
	{ 4514, 4050 },
	{ 4573, 4100 },
	{ 4628, 4150 },
	{ 4689, 4200 },
	{ 4797, 4300 },
	{ 4910, 4400 },
	{ 5020, 4500 },
};

static void jwaoo_charge_isr(struct jwaoo_irq_desc *desc, bool status)
{
	jwaoo_battery_poll_start();
}

struct jwaoo_irq_desc jwaoo_charge = {
	.port = CHG_DET_GPIO_PORT,
	.pin = CHG_DET_GPIO_PIN,
	.handler = jwaoo_charge_isr,
};

void jwaoo_battery_set_enable(bool enable)
{
	if (enable) {
		jwaoo_hw_irq_enable(CHG_DET_GPIO_IRQ, &jwaoo_charge, CHG_DET_ACTIVE_LOW);
	} else {
		jwaoo_hw_irq_disable(CHG_DET_GPIO_IRQ);
	}
}

void jwaoo_battery_led_blink(void)
{
	if (jwaoo_app_env.battery_led_locked < 2) {
		jwaoo_app_env.battery_led_locked = 1;
		jwaoo_pwm_blink_square_full(JWAOO_PWM_BATT_LED, 50, 1);
	}
}

void jwaoo_battery_led_release(uint8_t level)
{
	if (jwaoo_app_env.battery_led_locked <= level) {
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
}

void jwaoo_battery_set_state(uint8_t state)
{
	if (jwaoo_app_env.battery_state != state) {
		jwaoo_app_env.battery_state = state;
		jwaoo_battery_led_update_state(false);
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
	bool charge_online = CHG_ONLINE;

#if DEBUG_AVG_VOLTAGE
	jwaoo_app_timer_set(JWAOO_BATT_POLL_TIMER, 5);
#else
	jwaoo_app_timer_set(JWAOO_BATT_POLL_TIMER, JWAOO_BATT_POLL_DELAY);
#endif

	adc_calibrate();

	SetWord16(GP_ADC_CTRL_REG, GP_ADC_LDO_EN | GP_ADC_SE | GP_ADC_EN | ADC_CHANNEL_P01 << 6);
	SetWord16(GP_ADC_CTRL2_REG, GP_ADC_DELAY_EN | GP_ADC_I20U | GP_ADC_ATTN3X);

	for (i = 0, voltage = 0; i < 12; i++) {
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
	} else {
		jwaoo_app_env.battery_full = 0;

		if (level > JWAOO_BATT_LEVEL_NORMAL) {
			state = JWAOO_TOY_BATTERY_NORMAL;
		} else if (level > JWAOO_BATT_LEVEL_LOW) {
			state = jwaoo_app_env.battery_state_raw;
		} else {
			state = JWAOO_TOY_BATTERY_LOW;

			if (voltage < jwaoo_app_settings.shutdown_voltage) {
				jwaoo_app_goto_suspend_mode();
			}
		}
	}

	jwaoo_app_env.battery_level = level;
	jwaoo_app_env.battery_state_raw = state;
	jwaoo_battery_set_state(state);

	if (jwaoo_app_env.battery_report) {
		SEND_EMPTY_MESSAGE(JWAOO_TOY_BATT_REPORT_STATE, TASK_JWAOO_TOY);
	}
#endif
}

