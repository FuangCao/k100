#include "jwaoo_hw.h"
#include "jwaoo_key.h"
#include "jwaoo_pwm.h"
#include "jwaoo_spi.h"
#include "jwaoo_i2c.h"
#include "jwaoo_battery.h"

void jwaoo_hw_config_irq(IRQn_Type irq, GPIO_handler_function_t isr, GPIO_PORT port, GPIO_PIN pin)
{
	GPIO_RegisterCallback(irq, isr);
	GPIO_EnableIRQ(port, pin, irq, GPIO_GetPinStatus(port, pin), false, 60);
}

void jwaoo_hw_init(void)
{
	jwaoo_key_init();
	jwaoo_pwm_init();
	jwaoo_battery_init();

	// jwaoo_spi_init();
	// jwaoo_i2c_init(JWAOO_I2C_SPEED_400K, JWAOO_I2C_ADDRESS_MODE_7BIT);
}
