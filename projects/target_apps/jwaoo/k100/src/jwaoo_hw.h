#pragma once

#include "rwip.h"
#include "rwip_config.h"             // SW configuration
#include "wkupct_quadec.h"
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

#define JWAOO_IRQ_COUNT			(GPIO4_IRQn - GPIO0_IRQn + 1)

#define println(fmt, args...)

struct jwaoo_irq_desc
{
	GPIO_PORT port;
	GPIO_PIN pin;
	bool active_low;

	void (*handler)(struct jwaoo_irq_desc *desc, bool status);
};

struct jwaoo_device_data
{
	uint8_t bd_addr[6];
};

extern struct jwaoo_irq_desc *jwaoo_irqs[JWAOO_IRQ_COUNT];

bool jwaoo_hw_is_valid_bd_addr(const uint8_t bd_addr[6]);
bool jwaoo_hw_get_rand_bd_addr(uint8_t bd_addr[6]);
bool jwaoo_hw_irq_enable(IRQn_Type irq, struct jwaoo_irq_desc *desc, bool active_low);
bool jwaoo_hw_irq_disable(IRQn_Type irq);

void jwaoo_hw_init(void);
void jwaoo_hw_set_suspend(bool enable);
void jwaoo_hw_set_deep_sleep(bool enable);
void jwaoo_hw_set_device_enable(bool enable);

static bool jwaoo_hw_irq_invalid(IRQn_Type irq)
{
	return irq < GPIO0_IRQn || irq > GPIO4_IRQn;
}

static uint8_t jwaoo_hw_get_irq_index(IRQn_Type irq)
{
	return irq - GPIO0_IRQn;
}

static struct jwaoo_irq_desc *jwaoo_hw_get_irq_desc(IRQn_Type irq)
{
	return jwaoo_irqs[jwaoo_hw_get_irq_index(irq)];
}

static void jwaoo_hw_set_irq_desc(IRQn_Type irq, struct jwaoo_irq_desc *desc)
{
	jwaoo_irqs[jwaoo_hw_get_irq_index(irq)] = desc;
}
