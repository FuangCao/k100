#pragma once

#include "jwaoo_hw.h"
#include "jwaoo_app.h"

#define JWAOO_KEY_REPEAT_LONG_DELAY			100
#define JWAOO_KEY_REPEAT_SHORT_DELAY		10
#define JWAOO_KEY_LONG_CLICK_DELAY			300
#define JWAOO_KEY_MULTI_CLICK_DELAY			30

#define WAKEUP_KEY_SELECT(port, pin) \
	WKUPCT_PIN_SELECT(port, pin)

#define WAKEUP_KEY_POLARITY(port, pin) \
	WKUPCT_PIN_POLARITY(port, pin, KEY_ACTIVE_LOW)

#define WAKEUP_KEY_PORT			KEY1_GPIO_PORT
#define WAKEUP_KEY_PIN			KEY1_GPIO_PIN
#define WAKEUP_KEY_ACTIVE		KEY_GET_STATUS(WAKEUP_KEY_PORT, WAKEUP_KEY_PIN)

#define WAKEUP_KEY_MASK_BUILD(pin_mask, pol_mask, port, pin) \
	do { \
		(pin_mask) |= WKUPCT_PIN_SELECT(port, pin); \
		(pol_mask) |= WKUPCT_PIN_POLARITY(port, pin, GPIO_GetPinStatus(port, pin)); \
	} while (0)

#define WAKEUP_KEY_MASK_BUILD_ALL(pin_mask, pol_mask) \
	do { \
		(pin_mask) = 0; \
		(pol_mask) = 0; \
		WAKEUP_KEY_MASK_BUILD(pin_mask, pol_mask, WAKEUP_KEY_PORT, WAKEUP_KEY_PIN); \
		WAKEUP_KEY_MASK_BUILD(pin_mask, pol_mask, CHG_DET_GPIO_PORT, CHG_DET_GPIO_PIN); \
	} while (0)

enum {
	JWAOO_KEY_UP,
	JWAOO_KEY_O,
	JWAOO_KEY_DOWN,
	JWAOO_KEY_MAX,
};

enum {
	JWAOO_KEY_VALUE_UP,
	JWAOO_KEY_VALUE_DOWN,
	JWAOO_KEY_VALUE_REPEAT,
	JWAOO_KEY_VALUE_LONG,
};

struct jwaoo_key_device {
	struct jwaoo_irq_desc irq_desc;

	uint8_t code;
	uint8_t value;
	uint8_t count;
	uint8_t repeat;
	uint8_t last_value;
	bool lock_enable;
	bool repeat_enable;
	bool long_click_enable;
	bool multi_click_enable;
};

extern struct jwaoo_key_device jwaoo_keys[];

void jwaoo_key_init(void);
void jwaoo_key_process_active(uint8_t keycode);
void jwaoo_key_process_factory(uint8_t keycode);
void jwaoo_key_process_suspend(uint8_t keycode);

void jwaoo_key_lock_fire(void);
void jwaoo_key_repeat_fire(ke_msg_id_t const msgid, uint8_t keycode);
void jwaoo_key_long_click_fire(ke_msg_id_t const msgid, uint8_t keycode);
void jwaoo_key_multi_click_fire(ke_msg_id_t const msgid, uint8_t keycode);

static ke_msg_id_t jwaoo_key_get_repeat_timer(uint8_t keycode)
{
	return JWAOO_KEY1_REPEAT_TIMER + keycode;
}

static ke_msg_id_t jwaoo_key_get_long_click_timer(uint8_t keycode)
{
	return JWAOO_KEY1_LONG_CLICK_TIMER + keycode;
}

static ke_msg_id_t jwaoo_key_get_multi_click_timer(uint8_t keycode)
{
	return JWAOO_KEY1_MULTI_CLICK_TIMER + keycode;
}
