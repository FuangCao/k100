#include "jwaoo_app.h"
#include "jwaoo_moto.h"
#include "jwaoo_event.h"

void jwaoo_on_host_key_clicked(struct jwaoo_key_device *key, uint8_t count)
{
	switch (key->code) {
	case JWAOO_KEY_UP:
		jwaoo_moto_speed_add();
		break;

	case JWAOO_KEY_DOWN:
		if (jwaoo_moto_speed_sub() == 0 && key->repeat > 0) {
			jwaoo_app_goto_deep_sleep_mode();
		}
		break;

	case JWAOO_KEY_O:
		jwaoo_moto_mode_add();
		break;
	}
}

void jwaoo_on_host_key_long_clicked(struct jwaoo_key_device *key)
{
}

// ================================================================================

void jwaoo_on_client_key_state_changed(struct jwaoo_key_device *key)
{
}

void jwaoo_on_client_key_clicked(struct jwaoo_key_device *key, uint8_t count)
{
}

void jwaoo_on_client_key_long_clicked(struct jwaoo_key_device *key)
{
}

// ================================================================================

void jwaoo_on_battery_state_changed(void)
{
}
