#pragma once

#include "jwaoo_hw.h"

#define JWAOO_BATT_LEVEL_LOW		8
#define JWAOO_BATT_VOLTAGE_MIN		3000
#define JWAOO_BATT_VOLTAGE_MAX		4200
#define JWAOO_VOLTAGE_ARRAY_SIZE	20

extern struct jwaoo_irq_desc jwaoo_charge;

void jwaoo_set_battery_enable(bool enable);
void jwaoo_battery_led_blink(void);
void jwaoo_battery_led_release(uint8_t level);
void jwaoo_battery_led_update_state(void);
void jwaoo_battery_set_state(uint8_t state);
void jwaoo_battery_poll(void);
