#include "jwaoo_hw.h"
#include "jwaoo_key.h"
#include "jwaoo_pwm.h"
#include "jwaoo_spi.h"
#include "jwaoo_i2c.h"
#include "jwaoo_battery.h"
#include "jwaoo_app.h"

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
	}
}

void jwaoo_hw_config_irq(IRQn_Type irq, GPIO_handler_function_t isr, GPIO_PORT port, GPIO_PIN pin)
{
	GPIO_RegisterCallback(irq, isr);
	GPIO_EnableIRQ(port, pin, irq, GPIO_GetPinStatus(port, pin), false, 60);
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
