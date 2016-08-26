#include "jwaoo_app.h"
#include "jwaoo_key.h"

struct jwaoo_key_device jwaoo_keys[] = {
	{
		.irq_desc = {
			.port = KEY1_GPIO_PORT,
			.pin = KEY1_GPIO_PIN,
		},
	}, {
		.irq_desc = {
			.port = KEY2_GPIO_PORT,
			.pin = KEY2_GPIO_PIN,
		},
	}, {
		.irq_desc = {
			.port = KEY3_GPIO_PORT,
			.pin = KEY3_GPIO_PIN,
		}
	}, {
		.irq_desc = {
			.port = KEY4_GPIO_PORT,
			.pin = KEY4_GPIO_PIN,
		},
	},
};

static void jwaoo_key_isr(struct jwaoo_irq_desc *desc)
{
	if (desc->status) {
		BATT_LED_OPEN;
	} else {
		BATT_LED_CLOSE;
	}
}

void jwaoo_key_init(void)
{
	int i;

	for (i = 0; i < NELEM(jwaoo_keys); i++) {
		struct jwaoo_key_device *key = jwaoo_keys + i;
		struct jwaoo_irq_desc *desc = &key->irq_desc;

		key->code = i;
		desc->handler = jwaoo_key_isr;

		KEY_GPIO_CONFIG(desc->port, desc->pin);
		jwaoo_hw_irq_enable((IRQn_Type) (GPIO0_IRQn + i), desc);
	}

	jwaoo_keys[JWAOO_KEYCODE_UP].repeat_enable = true;
	jwaoo_keys[JWAOO_KEYCODE_DOWN].repeat_enable = true;
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

static void jwaoo_key_timer_clear(uint8_t code)
{
	ke_timer_clear(jwaoo_key_get_repeat_timer(code), TASK_JWAOO_APP);
	ke_timer_clear(jwaoo_key_get_long_click_timer(code), TASK_JWAOO_APP);
	ke_timer_clear(jwaoo_key_get_multi_click_timer(code), TASK_JWAOO_APP);
}

void jwaoo_key_process(const struct jwaoo_key_event *event)
{
#if 0
	struct jwaoo_key_device *key = jwaoo_keys + event->code;

	jwaoo_key_timer_clear(event->code);

	if (event->value) {
		key->last_value = key->value;
	}

	key->value = event->value;

	jwaoo_app_update_suspend_timer();

	if (jwaoo_toy_process_key_lock()) {
		return;
	}

	if (jwaoo_toy_env.key_locked) {
		return;
	}

	if (key->skip > 0) {
		if (value == 0) {
			key->skip--;
		}

		return;
	}

	if (value > 0) {
		key->count++;
		key->repeat = 0;
		ke_timer_set(key->repeat_timer, TASK_APP, JWAOO_TOY_KEY_REPEAT_LONG);
		ke_timer_set(key->long_click_timer, TASK_APP, jwaoo_toy_env.key_long_click_delay);
	} else {
		ke_timer_set(key->multi_click_timer, TASK_APP, jwaoo_toy_env.key_multi_click_delay);
	}

	if (key->multi_click_enable == false && (key->repeat_enable == false || key->repeat == 0)) {
		if (key->long_click_enable || key->lock_enable) {
			if (value == 0 && key->last_value != JWAOO_TOY_KEY_VALUE_LONG) {
				jwaoo_toy_on_key_clicked(key, 1);
			}
		} else if (value > 0) {
			jwaoo_toy_on_key_clicked(key, 1);
		}
	}

	if (!jwaoo_toy_env.key_multi_click_enable) {
		if (jwaoo_toy_env.key_long_click_enable) {
			if (value == 0 && key->last_value != JWAOO_TOY_KEY_VALUE_LONG) {
				jwaoo_toy_report_key_click(key, 1);
			}
		} else if (jwaoo_toy_env.key_click_enable) {
			if (value > 0) {
				jwaoo_toy_report_key_click(key, 1);
			}
		} else {
			jwaoo_toy_report_key_state(key, value);
		}
	}
#endif
}
