#include "adc.h"
#include "jwaoo_app.h"
#include "jwaoo_pwm.h"
#include "jwaoo_battery.h"
#include "jwaoo_toy_task.h"

#define DEBUG_AVG_VOLTAGE		0

static const struct jwaoo_battery_voltage_map jwaoo_battery_voltage_table[] = {
	{ 2063, 1800 },
	{ 2121, 1850 },
	{ 2177, 1900 },
	{ 2237, 1950 },
	{ 2295, 2000 },
	{ 2353, 2050 },
	{ 2411, 2100 },
	{ 2469, 2150 },
	{ 2526, 2200 },
	{ 2585, 2250 },
	{ 2643, 2300 },
	{ 2700, 2350 },
	{ 2757, 2400 },
	{ 2814, 2450 },
	{ 2873, 2500 },
	{ 2931, 2550 },
	{ 2991, 2600 },
	{ 3047, 2650 },
	{ 3105, 2700 },
	{ 3163, 2750 },
	{ 3221, 2800 },
	{ 3278, 2850 },
	{ 3334, 2900 },
	{ 3394, 2950 },
	{ 3452, 3000 },
	{ 3512, 3050 },
	{ 3566, 3100 },
	{ 2626, 3150 },
	{ 3685, 3200 },
	{ 3741, 3250 },
	{ 3800, 3300 },
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
#ifdef BATT_LED_GPIO_PORT
		jwaoo_pwm_blink_square_full(JWAOO_PWM_BATT_LED, 50, 1);
#endif
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

#ifdef BATT_LED_GPIO_PORT
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
#endif
}

void jwaoo_battery_set_state(uint8_t state)
{
	if (jwaoo_app_env.battery_state != state) {
		jwaoo_app_env.battery_state = state;
		jwaoo_battery_led_update_state(false);
	}
}

uint16_t jwaoo_battery_voltage_calibration(const struct jwaoo_battery_voltage_map *table, uint8_t size, volatile uint32_t voltage)
{
	const struct jwaoo_battery_voltage_map *map, *map_end;

	for (map = table, map_end = map + size - 1; map < map_end && map->raw_value < voltage; map++);

	return voltage * map->real_value / map->raw_value + 18;
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

#if DEBUG_AVG_VOLTAGE
	if (jwaoo_app_env.battery_skip < 3) {
		jwaoo_app_env.battery_skip++;
		return;
	}

	if (avg_count++ > 0) {
		avg_voltage = (avg_voltage * 7 + voltage) >> 3;

		if (avg_count > 200) {
			avg_count = 0;
		}
	} else {
		avg_voltage = voltage;
	}
#endif

	println("raw voltage = %d", voltage);

	voltage = jwaoo_battery_voltage_calibration(jwaoo_battery_voltage_table, NELEM(jwaoo_battery_voltage_table), voltage);

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

			if (voltage < jwaoo_app_settings.shutdown_voltage) {
				// jwaoo_app_goto_suspend_mode();
			}
		}
	}

	jwaoo_app_env.battery_level = level;
	jwaoo_battery_set_state(state);

	if (jwaoo_app_env.battery_report) {
		SEND_EMPTY_MESSAGE(JWAOO_TOY_BATT_REPORT_STATE, TASK_JWAOO_TOY);
	}
}

