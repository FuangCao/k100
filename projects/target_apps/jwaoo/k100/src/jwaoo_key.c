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
	return false;
}

void jwaoo_key_process_active(uint8_t keycode)
{
	struct jwaoo_key_device *key = jwaoo_keys + keycode;

	jwaoo_key_timer_clear(keycode);
	jwaoo_app_suspend_counter_reset();

	if (jwaoo_key_check_lock_state() || jwaoo_app_env.key_app_locked) {
		return;
	}

	if (key->value > 0) {
		key->count++;
		key->repeat = 0;
		jwaoo_app_timer_set(jwaoo_key_get_repeat_timer(keycode), JWAOO_KEY_REPEAT_LONG_DELAY);
		jwaoo_app_timer_set(jwaoo_key_get_long_click_timer(keycode), jwaoo_app_env.key_long_click_delay);
	} else {
		jwaoo_app_timer_set(jwaoo_key_get_multi_click_timer(keycode), jwaoo_app_env.key_multi_click_delay);
	}

	if (key->multi_click_enable == false) {
		if (key->repeat_enable) {
			if (key->repeat == 0 && key->value == 0) {
				jwaoo_on_host_key_clicked(key, 1);
			}
		} else if (key->long_click_enable) {
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
			jwaoo_on_client_key_state_changed(key, false);
		}
	}
}

void jwaoo_key_process_factory(uint8_t keycode)
{
	jwaoo_on_client_key_state_changed(jwaoo_keys + keycode, true);
}

void jwaoo_key_process_suspend(uint8_t keycode)
{
	if (jwaoo_app_env.key_locked && jwaoo_key_check_lock_state()) {
		return;
	}

	if (jwaoo_keys[0].value) {
		jwaoo_app_env.key_release_pending = true;
		jwaoo_app_goto_active_mode();
	}
}

bool jwaoo_key_check_release(void)
{
	bool success = true;
	struct jwaoo_key_device *key;
	struct jwaoo_key_device *key_end = jwaoo_keys + NELEM(jwaoo_keys);

	for (key = jwaoo_keys; key < key_end; key++) {
		key->value = jwaoo_hw_get_irq_status(&key->irq_desc);
		if (key->value && key->wait_release) {
			success = false;
		}
	}

	return success;
}

static void jwaoo_key_isr(struct jwaoo_irq_desc *desc, bool status)
{
	uint8_t *param;
	struct jwaoo_key_device *key = (struct jwaoo_key_device *) desc;

	if (jwaoo_app_env.key_release_pending) {
		if (jwaoo_key_check_release()) {
			jwaoo_app_env.key_release_pending = false;
			jwaoo_app_env.key_lock_pending = false;
			jwaoo_app_timer_clear(JWAOO_KEY_LOCK_TIMER);
			jwaoo_battery_led_release(2);
		}

		return;
	}

	if (key->value == status) {
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

void jwaoo_key_set_enable(bool enable)
{
	if (!jwaoo_app_env.initialized) {
		jwaoo_app_env.key_long_click_delay = JWAOO_KEY_LONG_CLICK_DELAY;
		jwaoo_app_env.key_multi_click_delay = JWAOO_KEY_MULTI_CLICK_DELAY;

		jwaoo_keys[0].repeat_enable = true;
		jwaoo_keys[0].wait_release = true;
		jwaoo_keys[0].report_enable = true;
	}

	if (enable) {
		for (int i = 0; i < NELEM(jwaoo_keys); i++) {
			struct jwaoo_key_device *key = jwaoo_keys + i;
			struct jwaoo_irq_desc *desc = &key->irq_desc;

			key->code = i;
			KEY_GPIO_CONFIG(desc->port, desc->pin);

			desc->handler = jwaoo_key_isr;
			jwaoo_hw_irq_enable((IRQn_Type) (GPIO0_IRQn + i), desc, KEY_ACTIVE_LOW);

			key->value = jwaoo_hw_get_irq_status(desc);
		}
	} else {
		for (int i = 0; i < NELEM(jwaoo_keys); i++) {
			jwaoo_hw_irq_disable((IRQn_Type) (GPIO0_IRQn + i));
		}
	}
}

bool jwaoo_key_get_status(uint8_t keycode)
{
	struct jwaoo_key_device *key = jwaoo_keys + keycode;

	key->value = jwaoo_hw_get_irq_status(&key->irq_desc);

	return key->value;
}

// ================================================================================

void jwaoo_key_lock_timer_fire(void)
{
	if (jwaoo_app_env.key_locked) {
		jwaoo_pwm_blink_open(JWAOO_PWM_BATT_LED);
		jwaoo_app_env.key_locked = false;
		jwaoo_app_goto_active_mode();
	} else {
		jwaoo_pwm_blink_close(JWAOO_PWM_BATT_LED);
		jwaoo_app_env.key_locked = true;

		if (!jwaoo_app_env.connected) {
			jwaoo_app_goto_suspend_mode();
		}
	}
}

void jwaoo_key_repeat_timer_fire(ke_msg_id_t const msgid, uint8_t keycode)
{
	struct jwaoo_key_device *key = jwaoo_keys + keycode;

	if (key->value > 0 && key->repeat < 0xFF) {
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

void jwaoo_key_long_click_timer_fire(ke_msg_id_t const msgid, uint8_t keycode)
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

void jwaoo_key_multi_click_timer_fire(ke_msg_id_t const msgid, uint8_t keycode)
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
