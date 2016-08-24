#include "gpio.h"
#include "jwaoo_battery.h"
#include "user_periph_setup.h"

struct jwaoo_battery_data jwaoo_battery_env __attribute__((section("retention_mem_area0"), zero_init));

static void jwaoo_charge_isr(void)
{
	bool status = GPIO_GetPinStatus(CHG_STAT_GPIO_PORT, CHG_STAT_GPIO_PIN);

	GPIO_SetIRQInputLevel(CHG_STAT_GPIO_IRQ, (GPIO_IRQ_INPUT_LEVEL) status);
	jwaoo_battery_env.charge_online = !status;
}

void jwaoo_battery_init(void)
{
	jwaoo_hw_config_irq(CHG_STAT_GPIO_IRQ, jwaoo_charge_isr, CHG_STAT_GPIO_PORT, CHG_STAT_GPIO_PIN);
}
