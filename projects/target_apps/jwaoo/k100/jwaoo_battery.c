#include "jwaoo_app.h"
#include "jwaoo_battery.h"

struct jwaoo_battery_data jwaoo_battery_env __attribute__((section("retention_mem_area0"), zero_init));

static void jwaoo_charge_isr(struct jwaoo_irq_desc *desc)
{
#if CHG_DET_ACTIVE_LOW
	jwaoo_battery_env.charge_online = !desc->status;
#else
	jwaoo_battery_env.charge_online = desc->status;
#endif

	if (!ke_timer_active(JWAOO_BATT_POLL, TASK_JWAOO_APP)) {
		ke_timer_set(JWAOO_BATT_POLL, TASK_JWAOO_APP, 1);
	}
}

static struct jwaoo_irq_desc jwaoo_charge_irq_desc = {
	.port = CHG_DET_GPIO_PORT,
	.pin = CHG_DET_GPIO_PIN,
	.handler = jwaoo_charge_isr,
};

void jwaoo_battery_init(void)
{
	jwaoo_hw_irq_enable(CHG_DET_GPIO_IRQ, &jwaoo_charge_irq_desc);
}
