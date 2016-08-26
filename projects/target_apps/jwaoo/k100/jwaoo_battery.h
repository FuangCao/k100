#pragma once

#include "jwaoo_hw.h"

extern struct jwaoo_irq_desc jwaoo_charge;

void jwaoo_battery_init(void);
void jwaoo_battery_led_release(void);
