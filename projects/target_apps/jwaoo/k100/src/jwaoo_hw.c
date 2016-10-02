#include "jwaoo_hw.h"
#include "jwaoo_key.h"
#include "jwaoo_pwm.h"
#include "jwaoo_spi.h"
#include "jwaoo_i2c.h"
#include "jwaoo_app.h"
#include "jwaoo_moto.h"
#include "jwaoo_battery.h"

extern uint8_t app_connection_idx;

struct jwaoo_irq_desc *jwaoo_irqs[JWAOO_IRQ_COUNT];

static void jwaoo_hw_gpio_isr(IRQn_Type irq)
{
	struct jwaoo_irq_desc *desc;

	GPIO_ResetIRQ(irq);

	desc = jwaoo_hw_get_irq_desc(irq);
	if (desc != NULL) {
		bool status = GPIO_GetPinStatus(desc->port, desc->pin);

		GPIO_SetIRQInputLevel(irq, (GPIO_IRQ_INPUT_LEVEL) status);
		desc->handler(desc, status ^ desc->active_low);
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
		jwaoo_moto_blink_close();
		app_easy_gap_disconnect(app_connection_idx);
		app_easy_gap_advertise_stop();
		jwaoo_pwm_blink_close(JWAOO_PWM_BT_LED);
	} else {
		jwaoo_app_adv_start();
	}
}

void jwaoo_hw_set_deep_sleep(bool enable)
{
	if (enable) {
		jwaoo_battery_poll_stop();
		jwaoo_pwm_blink_close(JWAOO_PWM_BATT_LED);

		arch_ble_ext_wakeup_on();

#if (USE_MEMORY_MAP == EXT_SLEEP_SETUP)
		arch_set_sleep_mode(ARCH_EXT_SLEEP_ON);
#else
		arch_set_sleep_mode(ARCH_DEEP_SLEEP_ON);
#endif

		jwaoo_hw_set_device_enable(false);
	} else {
		arch_ble_ext_wakeup_off();
		arch_set_sleep_mode(ARCH_SLEEP_OFF);

		if (GetBits16(SYS_STAT_REG, PER_IS_DOWN)) {
			periph_init();
		}

		jwaoo_hw_set_device_enable(true);
		arch_ble_force_wakeup();

		jwaoo_app_env.battery_led_locked = 0;
		jwaoo_battery_led_update_state();
		jwaoo_battery_poll_start();
	}
}

void jwaoo_hw_set_device_enable(bool enable)
{
	if (enable && jwaoo_app_env.device_enabled) {
		return;
	}

	jwaoo_app_env.device_enabled = enable;

	jwaoo_key_set_enable(enable);
	jwaoo_battery_set_enable(enable);

	jwaoo_spi_set_enable(enable);
	jwaoo_i2c_set_enable(enable);
}

void jwaoo_hw_init(void)
{
	if (jwaoo_app_env.initialized) {
		jwaoo_hw_set_device_enable(false);
	} else {
		jwaoo_hw_set_device_enable(true);
		jwaoo_spi_load_data();
	}
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
