#include "jwaoo_app.h"
#include "jwaoo_battery.h"

static void jwaoo_charge_isr(struct jwaoo_irq_desc *desc)
{
	jwaoo_app_env.charge_online = desc->status;

	if (!jwaoo_app_timer_active(JWAOO_BATT_POLL)) {
		jwaoo_app_timer_set(JWAOO_BATT_POLL, 1);
	}
}

struct jwaoo_irq_desc jwaoo_charge = {
	.port = CHG_DET_GPIO_PORT,
	.pin = CHG_DET_GPIO_PIN,
	.handler = jwaoo_charge_isr,
};

void jwaoo_battery_init(void)
{
	jwaoo_app_env.charge_online = CHG_ONLINE;
	jwaoo_hw_irq_enable(CHG_DET_GPIO_IRQ, &jwaoo_charge, CHG_DET_ACTIVE_LOW);
}

void jwaoo_battery_led_release(void)
{
}
