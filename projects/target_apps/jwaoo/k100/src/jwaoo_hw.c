#include "co_math.h"
#include "jwaoo_hw.h"
#include "jwaoo_key.h"
#include "jwaoo_pwm.h"
#include "jwaoo_i2c.h"
#include "jwaoo_app.h"
#include "jwaoo_moto.h"
#include "jwaoo_battery.h"

extern uint8_t app_connection_idx;

struct jwaoo_irq_desc *jwaoo_irqs[JWAOO_IRQ_COUNT];

bool jwaoo_hw_is_valid_bd_addr(const uint8_t bd_addr[6])
{
	uint8_t i;

	for (i = 0; i < 6; i++) {
		if (bd_addr[i] != 0x00 && bd_addr[i] != 0xFF) {
			return true;
		}
	}

	return false;
}

bool jwaoo_hw_get_rand_bd_addr(uint8_t bd_addr[6])
{
	uint8_t count;
	bool valid = false;

	for (count = 0; count < 100; count++) {
		uint8_t i;

		init_rand_seed_from_trng();

		for (i = 0; i < 6; i++) {
			uint8_t value = rand();

			bd_addr[i] = value;

			if (value != 0x00 && value != 0xFF) {
				valid = true;
			}
		}

		if (valid) {
			break;
		}
	}

	return valid;
}

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
	GPIO_EnableIRQ(desc->port, desc->pin, irq, status, false, 10);

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
#ifdef CHG_ONLINE
		if (!CHG_ONLINE) {
#else
		{
#endif
			jwaoo_battery_set_state(JWAOO_TOY_BATTERY_NORMAL);
		}

		jwaoo_battery_led_release(0, true);

#ifdef CFG_JWAOO_PWM_MOTO
		jwaoo_moto_blink_close();
#endif

		app_easy_gap_disconnect(app_connection_idx);
		user_app_set_state(APP_DISCONNECTED);
		app_easy_gap_advertise_stop();

		BT_LED_CLOSE;
	} else {
		if (jwaoo_app_env.key_lock_pending) {
			jwaoo_app_env.battery_led_locked = 2;
#ifdef CFG_JWAOO_PWM_BATT_LED
			jwaoo_pwm_blink_open(JWAOO_PWM_BATT_LED);
#endif
		} else if (jwaoo_app_env.key_release_pending) {
			if (jwaoo_key_settings.led_blink_delay > 0) {
				jwaoo_battery_led_blink();
			} else {
				jwaoo_battery_led_set_enable(true);
			}

#ifdef CFG_JWAOO_PWM_MOTO
			jwaoo_moto_set_mode(JWAOO_MOTO_MODE_LINE);
#endif
		} else {
			jwaoo_battery_led_update_state(true);
		}

		jwaoo_app_adv_start();
        SetBits16(SYS_CTRL_REG, DEBUGGER_ENABLE, 1);
	}
}

void jwaoo_hw_set_deep_sleep(bool enable)
{
	if (enable) {
		jwaoo_battery_poll_stop();
		jwaoo_battery_set_state(JWAOO_TOY_BATTERY_NORMAL);

#ifdef CFG_JWAOO_PWM_MOTO
		jwaoo_moto_blink_close();
#endif

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
	jwaoo_i2c_set_enable(enable);
}

void jwaoo_hw_init(void)
{
	GPIO_ConfigurePin(SPI_CS_GPIO_PORT, SPI_CS_GPIO_PIN, OUTPUT, PID_GPIO, true);
	GPIO_ConfigurePin(SPI_CLK_GPIO_PORT, SPI_CLK_GPIO_PIN, INPUT, PID_GPIO, false);
	GPIO_ConfigurePin(SPI_DO_GPIO_PORT, SPI_DO_GPIO_PIN, INPUT, PID_GPIO, false);
	GPIO_ConfigurePin(SPI_DI_GPIO_PORT, SPI_DI_GPIO_PIN, INPUT, PID_GPIO, false);

	if (jwaoo_app_env.initialized) {
		jwaoo_hw_set_device_enable(false);
	} else {
		jwaoo_hw_set_device_enable(true);
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
