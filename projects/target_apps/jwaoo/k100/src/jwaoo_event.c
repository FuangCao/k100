#include "jwaoo_app.h"
#include "jwaoo_toy.h"
#include "jwaoo_moto.h"
#include "jwaoo_event.h"

void jwaoo_on_host_key_clicked(struct jwaoo_key_device *key, uint8_t count)
{
	switch (key->code) {
	case JWAOO_KEY_UP:
		jwaoo_moto_speed_add(1);
		break;

	case JWAOO_KEY_DOWN:
		if (jwaoo_moto_speed_add(-1)) {
			if (key->repeat > 1) {
				key->repeat = 1;
			}
		} else if (key->repeat > 20) {
			jwaoo_app_goto_suspend_mode();
		}
		break;

	case JWAOO_KEY_O:
		if (jwaoo_app_env.connected) {
			return;
		}

		jwaoo_moto_mode_add();
		break;

	default:
		return;
	}

	jwaoo_battery_led_blink();
}

void jwaoo_on_host_key_long_clicked(struct jwaoo_key_device *key)
{
}

// ================================================================================

void jwaoo_on_client_key_state_changed(struct jwaoo_key_device *key)
{
	jwaoo_toy_report_key_state(key);
}

void jwaoo_on_client_key_clicked(struct jwaoo_key_device *key, uint8_t count)
{
	jwaoo_toy_report_key_click(key, count);
}

void jwaoo_on_client_key_long_clicked(struct jwaoo_key_device *key)
{
	jwaoo_toy_report_key_long_click(key);
}
