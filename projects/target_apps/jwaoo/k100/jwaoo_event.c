#include "jwaoo_event.h"

void jwaoo_on_host_key_clicked(struct jwaoo_key_device *key, uint8_t count)
{
	static int click_count;

	if (++click_count & 1) {
		BATT_LED_OPEN;
	} else {
		BATT_LED_CLOSE;
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
