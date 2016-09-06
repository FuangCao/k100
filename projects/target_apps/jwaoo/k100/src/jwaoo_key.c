#include "jwaoo_app.h"
#include "jwaoo_key.h"
#include "jwaoo_pwm.h"
#include "jwaoo_event.h"
#include "jwaoo_battery.h"

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

static void jwaoo_key_timer_clear(uint8_t keycode)
{
	jwaoo_app_timer_clear(jwaoo_key_get_repeat_timer(keycode));
	jwaoo_app_timer_clear(jwaoo_key_get_long_click_timer(keycode));
	jwaoo_app_timer_clear(jwaoo_key_get_multi_click_timer(keycode));
}

static bool jwaoo_key_check_lock_state(void)
{
	struct jwaoo_key_device *key, *key_end;

	key_end = jwaoo_keys + NELEM(jwaoo_keys);

	for (key = jwaoo_keys; key < key_end; key++) {
		if (key->lock_enable && key->value == 0) {
			if (jwaoo_app_env.key_lock_pending) {
				jwaoo_app_env.key_lock_pending = false;
				jwaoo_app_timer_clear(JWAOO_KEY_LOCK_TIMER);
				jwaoo_battery_led_release();

				for (key = jwaoo_keys; key < key_end; key++) {
					key->value = 0;
				}
			}

			return jwaoo_app_env.key_locked;
		}
	}

	if (jwaoo_app_env.key_lock_pending) {
		return true;
	}

	jwaoo_app_env.key_lock_pending = true;

	for (key = jwaoo_keys; key < key_end; key++) {
		jwaoo_key_timer_clear(key->code);
	}

	jwaoo_app_env.battery_led_locked = 2;

	if (jwaoo_app_env.key_locked) {
		jwaoo_pwm_blink_close(JWAOO_PWM_BATT_LED);
	} else {
		jwaoo_pwm_blink_open(JWAOO_PWM_BATT_LED);
	}

	jwaoo_app_timer_set(JWAOO_KEY_LOCK_TIMER, 300);

	return true;
}

void jwaoo_key_process_active(uint8_t keycode)
{
	struct jwaoo_key_device *key = jwaoo_keys + keycode;

	jwaoo_key_timer_clear(keycode);

	if (jwaoo_key_check_lock_state()) {
		return;
	}

	jwaoo_app_update_suspend_timer();

	if (key->value > 0) {
		key->count++;
		key->repeat = 0;
		jwaoo_app_timer_set(jwaoo_key_get_repeat_timer(keycode), JWAOO_KEY_REPEAT_LONG_DELAY);
		jwaoo_app_timer_set(jwaoo_key_get_long_click_timer(keycode), jwaoo_app_env.key_long_click_delay);
	} else {
		jwaoo_app_timer_set(jwaoo_key_get_multi_click_timer(keycode), jwaoo_app_env.key_multi_click_delay);
	}

	if (key->multi_click_enable == false && (key->repeat_enable == false || key->repeat == 0)) {
		if (key->long_click_enable || key->lock_enable) {
			if (key->value == 0 && key->last_value != JWAOO_KEY_VALUE_LONG) {
				jwaoo_on_host_key_clicked(key, 1);
			}
		} else if (key->value > 0) {
			jwaoo_on_host_key_clicked(key, 1);
		}
	}

	if (!jwaoo_app_env.key_multi_click_enable) {
		if (jwaoo_app_env.key_long_click_enable) {
			if (key->value == 0 && key->last_value != JWAOO_KEY_VALUE_LONG) {
				jwaoo_on_client_key_clicked(key, 1);
			}
		} else if (jwaoo_app_env.key_click_enable) {
			if (key->value > 0) {
				jwaoo_on_client_key_clicked(key, 1);
			}
		} else {
			jwaoo_on_client_key_state_changed(key);
		}
	}
}

void jwaoo_key_process_factory(uint8_t keycode)
{
	jwaoo_on_client_key_state_changed(jwaoo_keys + keycode);
}

void jwaoo_key_process_suspend(uint8_t keycode)
{
	if (keycode == JWAOO_KEY_UP && jwaoo_keys[keycode].value) {
		jwaoo_app_goto_active_mode();
	}
}

static void jwaoo_key_isr(struct jwaoo_irq_desc *desc, bool status)
{
	uint8_t *param;
	struct jwaoo_key_device *key = (struct jwaoo_key_device *) desc;

	if (status == key->value) {
		return;
	}

	if (!status) {
		key->last_value = key->value;
	}

	key->value = status;

	param = jwaoo_app_msg_alloc(JWAOO_PROCESS_KEY, sizeof(uint8_t));
	*param = key->code;
	jwaoo_app_msg_send(param);
}

void jwaoo_key_init(void)
{
	int i;

	jwaoo_app_env.key_long_click_delay = JWAOO_KEY_LONG_CLICK_DELAY;
	jwaoo_app_env.key_multi_click_delay = JWAOO_KEY_MULTI_CLICK_DELAY;

	jwaoo_keys[JWAOO_KEY_UP].repeat_enable = true;
	jwaoo_keys[JWAOO_KEY_UP].lock_enable = true;
	jwaoo_keys[JWAOO_KEY_DOWN].repeat_enable = true;
	jwaoo_keys[JWAOO_KEY_DOWN].lock_enable = true;

	for (i = 0; i < NELEM(jwaoo_keys); i++) {
		struct jwaoo_key_device *key = jwaoo_keys + i;
		struct jwaoo_irq_desc *desc = &key->irq_desc;

		key->code = i;
		key->value = 0;
		desc->handler = jwaoo_key_isr;

		KEY_GPIO_CONFIG(desc->port, desc->pin);
		jwaoo_hw_irq_enable((IRQn_Type) (GPIO0_IRQn + i), desc, KEY_ACTIVE_LOW);
	}
}

// ================================================================================

void jwaoo_key_lock_fire(void)
{
	if (jwaoo_app_env.key_locked) {
		jwaoo_pwm_blink_open(JWAOO_PWM_BATT_LED);
		jwaoo_app_env.key_locked = false;
	} else {
		jwaoo_pwm_blink_close(JWAOO_PWM_BATT_LED);
		jwaoo_app_env.key_locked = true;
	}
}

void jwaoo_key_repeat_fire(ke_msg_id_t const msgid, uint8_t keycode)
{
	struct jwaoo_key_device *key = jwaoo_keys + keycode;

	if (key->value > 0) {
		jwaoo_app_timer_set(msgid, JWAOO_KEY_REPEAT_SHORT_DELAY);

		key->repeat++;

		if (key->value < JWAOO_KEY_VALUE_REPEAT) {
			key->value = JWAOO_KEY_VALUE_REPEAT;
		}

		if (key->repeat_enable) {
			jwaoo_on_host_key_clicked(key, key->count);
		}
	}
}

void jwaoo_key_long_click_fire(ke_msg_id_t const msgid, uint8_t keycode)
{
	struct jwaoo_key_device *key = jwaoo_keys + keycode;

	key->value = JWAOO_KEY_VALUE_LONG;

	if (jwaoo_app_env.key_long_click_enable) {
		jwaoo_on_client_key_long_clicked(key);
	}

	if (key->long_click_enable) {
		jwaoo_on_host_key_long_clicked(key);
	}
}

void jwaoo_key_multi_click_fire(ke_msg_id_t const msgid, uint8_t keycode)
{
	struct jwaoo_key_device *key = jwaoo_keys + keycode;

	if (jwaoo_app_env.key_long_click_enable == false || key->last_value != JWAOO_KEY_VALUE_LONG) {
		if (jwaoo_app_env.key_multi_click_enable) {
			jwaoo_on_client_key_clicked(key, key->count);
		}
	}

	if (key->long_click_enable == false || key->last_value != JWAOO_KEY_VALUE_LONG) {
		if (key->multi_click_enable) {
			jwaoo_on_host_key_clicked(key, key->count);
		}
	}

	key->count = 0;
}
