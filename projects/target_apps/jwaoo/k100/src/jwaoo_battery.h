#pragma once

#include "jwaoo_hw.h"

#define JWAOO_BATT_POLL_DELAY				100
#define JWAOO_BATT_LEVEL_LOW				8
#define JWAOO_BATT_VOLTAGE_MIN				2400
#define JWAOO_BATT_VOLTAGE_MAX				3000
#define JWAOO_BATT_VOLTAGE_VALID_MIN		1000
#define JWAOO_BATT_VOLTAGE_VALID_MAX		4000
#define JWAOO_BATT_VOLTAGE_SHUTDOWN			2000
#define JWAOO_BATT_VOLTAGE_SHUTDOWN_MAX		2800
#define JWAOO_VOLTAGE_ARRAY_SIZE			20

struct jwaoo_battery_voltage_map {
	uint16_t raw_value;
	uint16_t real_value;
};

extern struct jwaoo_irq_desc jwaoo_charge;

void jwaoo_battery_set_enable(bool enable);
void jwaoo_battery_led_blink(void);
void jwaoo_battery_led_release(uint8_t level);
void jwaoo_battery_led_update_state(bool force);
void jwaoo_battery_set_state(uint8_t state);
void jwaoo_battery_poll(bool optimize);
uint16_t jwaoo_battery_voltage_calibration(const struct jwaoo_battery_voltage_map *table, uint8_t size, uint32_t voltage);
