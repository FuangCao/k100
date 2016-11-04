#include "jwaoo_app.h"
#include "jwaoo_toy.h"
#include "jwaoo_moto.h"
#include "jwaoo_event.h"

void jwaoo_on_host_key_clicked(struct jwaoo_key_device *key, uint8_t count)
{
	jwaoo_battery_led_blink();

	switch (key->code) {
	case JWAOO_KEY_UP:
		jwaoo_moto_speed_add(1);
		break;

	case JWAOO_KEY_DOWN:
		if (jwaoo_moto_speed_add(-1)) {
			if (key->repeat > 1) {
				key->repeat = 1;
			}
		} else {
			if (jwaoo_app_env.moto_mode != JWAOO_MOTO_MODE_IDLE) {
				jwaoo_moto_blink_close();
			}

			if (key->repeat > 0) {
				if (key->repeat > 20) {
					jwaoo_app_goto_suspend_mode();
				}
			}
		}
		break;

	case JWAOO_KEY_O:
		if (jwaoo_app_env.connected) {
			break;
		}

		if (jwaoo_app_env.moto_mode > JWAOO_MOTO_MODE_IDLE) {
			jwaoo_moto_mode_add();
		}
		break;

	case JWAOO_KEY_MAX:
		SetBits16(SYS_CTRL_REG, DEBUGGER_ENABLE, 1);
		break;
	}
}

void jwaoo_on_host_key_long_clicked(struct jwaoo_key_device *key)
{
}

// ================================================================================

void jwaoo_on_client_key_state_changed(struct jwaoo_key_device *key, bool force)
{
	if (key->report_enable || force) {
		jwaoo_toy_report_key_state(key);
	}
}

void jwaoo_on_client_key_clicked(struct jwaoo_key_device *key, uint8_t count)
{
	if (key->report_enable) {
		jwaoo_toy_report_key_click(key, count);
	}
}

void jwaoo_on_client_key_long_clicked(struct jwaoo_key_device *key)
{
	if (key->report_enable) {
		jwaoo_toy_report_key_long_click(key);
	}
}
