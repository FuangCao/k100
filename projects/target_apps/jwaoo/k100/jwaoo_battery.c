#include "jwaoo_app.h"
#include "jwaoo_battery.h"

struct jwaoo_battery_data jwaoo_battery_env __attribute__((section("retention_mem_area0"), zero_init));

static void jwaoo_charge_isr(void)
{
	bool status = GPIO_GetPinStatus(CHG_DET_GPIO_PORT, CHG_DET_GPIO_PIN);

	GPIO_SetIRQInputLevel(CHG_DET_GPIO_IRQ, (GPIO_IRQ_INPUT_LEVEL) status);

#if CHG_DET_ACTIVE_LOW
	jwaoo_battery_env.charge_online = !status;
#endif

	if (!ke_timer_active(JWAOO_BATT_POLL, TASK_JWAOO_APP)) {
		ke_timer_set(JWAOO_BATT_POLL, TASK_JWAOO_APP, 1);
	}
}

void jwaoo_battery_init(void)
{
	jwaoo_hw_config_irq(CHG_DET_GPIO_IRQ, jwaoo_charge_isr, CHG_DET_GPIO_PORT, CHG_DET_GPIO_PIN);
}
