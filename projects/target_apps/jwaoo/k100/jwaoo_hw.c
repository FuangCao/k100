#include "jwaoo_hw.h"
#include "jwaoo_key.h"
#include "jwaoo_pwm.h"
#include "jwaoo_spi.h"
#include "jwaoo_i2c.h"
#include "jwaoo_battery.h"
#include "jwaoo_app.h"

struct jwaoo_irq_desc *jwaoo_irqs[JWAOO_IRQ_COUNT];

static inline bool jwaoo_hw_irq_get_status(struct jwaoo_irq_desc *desc, bool status)
{
	return status ^ desc->active_low;
}

static void jwaoo_hw_gpio_isr(IRQn_Type irq)
{
	struct jwaoo_irq_desc *desc;

	GPIO_ResetIRQ(irq);

	desc = jwaoo_hw_get_irq_desc(irq);
	if (desc != NULL) {
		bool status = GPIO_GetPinStatus(desc->port, desc->pin);
		GPIO_SetIRQInputLevel(irq, (GPIO_IRQ_INPUT_LEVEL) status);

		status = jwaoo_hw_irq_get_status(desc, status);
		if (status != desc->status) {
			desc->status = status;
			desc->handler(desc);
		}
	} else {
		NVIC_ClearPendingIRQ(irq);	   
	}
}

bool jwaoo_hw_irq_enable(IRQn_Type irq, struct jwaoo_irq_desc *desc, bool active_low)
{
	bool status;

	if (jwaoo_hw_irq_invalid(irq)) {
		return false;
	}

	if (desc->handler == NULL) {
		return false;
	}

	desc->active_low = active_low;
	jwaoo_hw_set_irq_desc(irq, desc);

	status = GPIO_GetPinStatus(desc->port, desc->pin);
	desc->status = jwaoo_hw_irq_get_status(desc, status);
	GPIO_EnableIRQ(desc->port, desc->pin, irq, status, false, 60);

	return true;
}

bool jwaoo_hw_irq_disable(IRQn_Type irq)
{
	if (jwaoo_hw_irq_invalid(irq)) {
		return false;
	}

	NVIC_DisableIRQ(irq);
	jwaoo_hw_set_irq_desc(irq, NULL);

	return true;
}

// ================================================================================

void jwaoo_hw_set_suspend(bool enable)
{
	if (enable) {
		jwaoo_app_adv_stop();
	} else {
		jwaoo_app_adv_start();
	}
}

void jwaoo_hw_set_deep_sleep(bool enable)
{
	if (enable) {
		jwaoo_app_timer_clear(JWAOO_BATT_POLL);

		arch_ble_ext_wakeup_on();

#if (USE_MEMORY_MAP == EXT_SLEEP_SETUP)
		arch_set_sleep_mode(ARCH_EXT_SLEEP_ON);
#else
		arch_set_sleep_mode(ARCH_DEEP_SLEEP_ON);
#endif

		jwaoo_hw_set_enable(false);
	} else {
		arch_ble_ext_wakeup_off();
		arch_set_sleep_mode(ARCH_SLEEP_OFF);

		if (GetBits16(SYS_STAT_REG, PER_IS_DOWN)) {
			periph_init();
		} else {
			jwaoo_hw_set_enable(true);
		}

		arch_ble_force_wakeup();
		SetBits16(SYS_CTRL_REG, DEBUGGER_ENABLE, 1);

		jwaoo_app_timer_set(JWAOO_BATT_POLL, 1);
	}
}

void jwaoo_hw_set_enable(bool enable)
{
	jwaoo_spi_set_enable(enable);
	jwaoo_i2c_set_enable(enable);
}

void jwaoo_hw_init(void)
{
	jwaoo_key_init();
	jwaoo_battery_init();
	jwaoo_hw_set_enable(jwaoo_app_not_deep_sleep());
}

// ================================================================================

void GPIO0_Handler(void)
{
	jwaoo_hw_gpio_isr(GPIO0_IRQn);
}

void GPIO1_Handler(void)
{
	jwaoo_hw_gpio_isr(GPIO1_IRQn);
}

void GPIO2_Handler(void)
{
	jwaoo_hw_gpio_isr(GPIO2_IRQn);
}

void GPIO3_Handler(void)
{
	jwaoo_hw_gpio_isr(GPIO3_IRQn);
}

void GPIO4_Handler(void)
{
	jwaoo_hw_gpio_isr(GPIO4_IRQn);
}
