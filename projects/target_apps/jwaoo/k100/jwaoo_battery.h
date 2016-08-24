#pragma once

#include "jwaoo_hw.h"

struct jwaoo_battery_data {
	bool charge_online;
};

extern struct jwaoo_battery_data jwaoo_battery_env;

void jwaoo_battery_init(void);
