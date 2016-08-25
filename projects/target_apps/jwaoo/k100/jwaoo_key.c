#include "jwaoo_app.h"
#include "jwaoo_key.h"

struct jwaoo_key_device jwaoo_keys[JWAOO_KEY_COUNT] = {
	{
		.code = 0,
		.repeat_timer = JWAOO_KEY1_REPEAT_TIMER,
		.long_click_timer = JWAOO_KEY1_LONG_CLICK_TIMER,
		.multi_click_timer = JWAOO_KEY1_MULTI_CLICK_TIMER,
	}, {
		.code = 1,
		.repeat_timer = JWAOO_KEY2_REPEAT_TIMER,
		.long_click_timer = JWAOO_KEY2_LONG_CLICK_TIMER,
		.multi_click_timer = JWAOO_KEY2_MULTI_CLICK_TIMER,
	}, {
		.code = 2,
		.repeat_timer = JWAOO_KEY3_REPEAT_TIMER,
		.long_click_timer = JWAOO_KEY3_LONG_CLICK_TIMER,
		.multi_click_timer = JWAOO_KEY3_MULTI_CLICK_TIMER,
	}, {
		.code = 3,
		.repeat_timer = JWAOO_KEY4_REPEAT_TIMER,
		.long_click_timer = JWAOO_KEY4_LONG_CLICK_TIMER,
		.multi_click_timer = JWAOO_KEY4_MULTI_CLICK_TIMER,
	}
};

static void jwaoo_key_process(IRQn_Type irq, uint8_t code, GPIO_PORT port, GPIO_PIN pin)
{
	bool status = GPIO_GetPinStatus(port, pin);

	GPIO_SetIRQInputLevel(irq, (GPIO_IRQ_INPUT_LEVEL) status);

#if KEY_ACTIVE_LOW
	status = !status;
#endif
}

static void jwaoo_key1_isr(void)
{
	jwaoo_key_process(KEY1_GPIO_IRQ, 0, KEY1_GPIO_PORT, KEY1_GPIO_PIN);
}

static void jwaoo_key2_isr(void)
{
	jwaoo_key_process(KEY2_GPIO_IRQ, 1, KEY2_GPIO_PORT, KEY2_GPIO_PIN);
}

static void jwaoo_key3_isr(void)
{
	jwaoo_key_process(KEY3_GPIO_IRQ, 2, KEY3_GPIO_PORT, KEY3_GPIO_PIN);
}

static void jwaoo_key4_isr(void)
{
	jwaoo_key_process(KEY4_GPIO_IRQ, 3, KEY4_GPIO_PORT, KEY4_GPIO_PIN);
}

void jwaoo_key_init(void)
{
	jwaoo_keys[JWAOO_KEYCODE_UP].repeat_enable = true;
	jwaoo_keys[JWAOO_KEYCODE_DOWN].repeat_enable = true;

	jwaoo_hw_config_irq(KEY1_GPIO_IRQ, jwaoo_key1_isr, KEY1_GPIO_PORT, KEY1_GPIO_PIN);
	jwaoo_hw_config_irq(KEY2_GPIO_IRQ, jwaoo_key2_isr, KEY2_GPIO_PORT, KEY2_GPIO_PIN);
	jwaoo_hw_config_irq(KEY3_GPIO_IRQ, jwaoo_key3_isr, KEY3_GPIO_PORT, KEY3_GPIO_PIN);
	jwaoo_hw_config_irq(KEY4_GPIO_IRQ, jwaoo_key4_isr, KEY4_GPIO_PORT, KEY4_GPIO_PIN);
}

void jwaoo_key_repeat_fire(uint8_t key)
{
}

void jwaoo_key_long_click_fire(uint8_t key)
{
}

void jwaoo_key_multi_click_fire(uint8_t key)
{
}

