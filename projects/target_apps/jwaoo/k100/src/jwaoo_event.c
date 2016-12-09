#include "jwaoo_app.h"
#include "jwaoo_toy.h"
#include "jwaoo_moto.h"
#include "jwaoo_event.h"

void jwaoo_on_host_key_clicked(struct jwaoo_key_device *key, uint8_t count)
{
	jwaoo_battery_led_blink();
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
