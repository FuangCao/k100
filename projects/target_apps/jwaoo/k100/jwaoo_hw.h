#pragma once

#include "rwip_config.h"             // SW configuration
#include "global_io.h"
#include "gpio.h"
#include "user_periph_setup.h"       // peripheral configuration

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) \
	(sizeof(a) / sizeof((a)[0]))
#endif

#ifndef NELEM
#define NELEM(a) \
	((int) ARRAY_SIZE(a))
#endif

void jwaoo_hw_init(void);
void jwaoo_hw_config_irq(IRQn_Type irq, GPIO_handler_function_t isr, GPIO_PORT port, GPIO_PIN pin);
