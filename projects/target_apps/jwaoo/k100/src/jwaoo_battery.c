#include "adc.h"
#include "jwaoo_app.h"
#include "jwaoo_pwm.h"
#include "jwaoo_battery.h"
#include "jwaoo_toy_task.h"

static const struct jwaoo_battery_voltage_map jwaoo_battery_voltage_table[] = {
	{ 3120, 2800 },
	{ 3230, 2900 },
	{ 3340, 3000 },
	{ 3450, 3100 },
	{ 3560, 3200 },
	{ 3680, 3300 },
	{ 3790, 3400 },
	{ 3900, 3500 },
	{ 4020, 3600 },
	{ 4130, 3700 },
	{ 4240, 3800 },
	{ 4360, 3900 },
	{ 4460, 4000 },
	{ 4580, 4100 },
	{ 4680, 4200 },
	{ 4790, 4300 },
	{ 4910, 4400 },
	{ 5030, 4500 },
};

static void jwaoo_charge_isr(struct jwaoo_irq_desc *desc, bool status)
{
	jwaoo_app_env.charge_online = status;
	jwaoo_battery_poll_start();
}

struct jwaoo_irq_desc jwaoo_charge = {
	.port = CHG_DET_GPIO_PORT,
	.pin = CHG_DET_GPIO_PIN,
	.handler = jwaoo_charge_isr,
};

void jwaoo_battery_set_enable(bool enable)
{
	jwaoo_app_env.charge_online = CHG_ONLINE;

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
		jwaoo_pwm_blink_square_full(JWAOO_PWM_BATT_LED, 50, 2);
	}
}

void jwaoo_battery_led_release(uint8_t level)
{
	if (jwaoo_app_env.battery_led_locked <= level) {
		jwaoo_app_env.battery_led_locked = 0;
		jwaoo_battery_led_update_state();
	}
}

void jwaoo_battery_led_update_state(void)
{
	if (jwaoo_app_env.battery_led_locked) {
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
		jwaoo_pwm_blink_sawtooth_full(JWAOO_PWM_BATT_LED, JWAOO_PWM_LEVEL_MAX / 10, 2000, 0);
		break;

	default:
		jwaoo_pwm_blink_close(JWAOO_PWM_BATT_LED);
	}
}

void jwaoo_battery_set_state(uint8_t state)
{
	if (jwaoo_app_env.battery_state != state) {
		jwaoo_app_env.battery_state = state;
		jwaoo_battery_led_update_state();
	}
}

uint16_t jwaoo_battery_voltage_calibration(const struct jwaoo_battery_voltage_map *table, uint8_t size, uint32_t voltage)
{
	const struct jwaoo_battery_voltage_map *map, *map_end;

	for (map = table, map_end = map + size; map < map_end; map++) {
		if (voltage > map->raw_value) {
			continue;
		}

		if (map > table) {
			uint16_t raw_min, raw_range;
			uint16_t real_min, real_range;
			const struct jwaoo_battery_voltage_map *prev = map - 1;

			raw_min = prev->raw_value;
			raw_range = map->raw_value - raw_min;

			real_min = prev->real_value;
			real_range = map->real_value - real_min;

			return (voltage - raw_min) * real_range / raw_range + real_min;
		} else {
			return voltage * map->real_value / map->raw_value;
		}
	}

	map = map_end - 1;

	return voltage * map->real_value / map->raw_value;
}

void jwaoo_battery_poll(bool optimize)
{
	int i;
	uint8_t state;
	uint8_t level;
	uint32_t voltage;

	jwaoo_app_timer_set(JWAOO_BATT_POLL_TIMER, JWAOO_BATT_POLL_DELAY);

	jwaoo_app_env.charge_online = CHG_ONLINE;

	adc_calibrate();

	SetWord16(GP_ADC_CTRL_REG, GP_ADC_LDO_EN | GP_ADC_SE | GP_ADC_EN | ADC_CHANNEL_P01 << 6);
	SetWord16(GP_ADC_CTRL2_REG, GP_ADC_DELAY_EN | GP_ADC_I20U | GP_ADC_ATTN3X);

	for (i = 0, voltage = 0; i < 12; i++) {
		SetBits16(GP_ADC_CTRL_REG, GP_ADC_SIGN, i & 1);
		voltage += adc_get_sample();
	}

	adc_disable();

	println("raw voltage = %d", voltage);

	voltage = jwaoo_battery_voltage_calibration(jwaoo_battery_voltage_table, NELEM(jwaoo_battery_voltage_table), voltage);

	if (optimize) {
		if (voltage < JWAOO_BATT_VOLTAGE_VALID_MIN || voltage > JWAOO_BATT_VOLTAGE_VALID_MAX) {
			voltage = JWAOO_BATT_VOLTAGE_MAX;
		} else {
			if (jwaoo_app_env.charge_online && voltage < 4226 && jwaoo_app_env.battery_state != JWAOO_TOY_BATTERY_FULL) {
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

	if (jwaoo_app_env.charge_online) {
		if (level < 100) {
			state = JWAOO_TOY_BATTERY_CHARGING;
		} else if (BATT_CHARGING) {
			level = 99;
			state = JWAOO_TOY_BATTERY_CHARGING;
		} else {
			state = JWAOO_TOY_BATTERY_FULL;
		}
	} else {
		if (level > JWAOO_BATT_LEVEL_LOW) {
			state = JWAOO_TOY_BATTERY_NORMAL;
		} else {
			state = JWAOO_TOY_BATTERY_LOW;

			if (voltage < jwaoo_app_settings.shutdown_voltage) {
				jwaoo_app_goto_suspend_mode();
			}
		}
	}

	jwaoo_app_env.battery_level = level;
	jwaoo_battery_set_state(state);

	if (jwaoo_app_env.battery_report) {
		SEND_EMPTY_MESSAGE(JWAOO_TOY_BATT_REPORT_STATE, TASK_JWAOO_TOY);
	}
}

