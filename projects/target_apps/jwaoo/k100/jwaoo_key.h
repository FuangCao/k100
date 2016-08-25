#pragma once

#include "jwaoo_hw.h"

#define JWAOO_KEY_COUNT			4

#define JWAOO_KEYCODE_UP		0
#define JWAOO_KEYCODE_O			1
#define JWAOO_KEYCODE_DOWN		2
#define JWAOO_KEYCODE_MAX		3

#define WAKEUP_KEY_SELECT(port, pin) \
	WKUPCT_PIN_SELECT(port, pin)

#define WAKEUP_KEY_POLARITY(port, pin) \
	WKUPCT_PIN_POLARITY(port, pin, KEY_ACTIVE_LOW)

#define WAKEUP_KEY_PORT			KEY1_GPIO_PORT
#define WAKEUP_KEY_PIN			KEY1_GPIO_PIN
#define WAKEUP_KEY_ACTIVE		KEY_IS_ACTIVE(WAKEUP_KEY_PORT, WAKEUP_KEY_PIN)

#define WAKEUP_KEY_SEL_MASK \
	( \
		WKUPCT_PIN_SELECT(WAKEUP_KEY_PORT, WAKEUP_KEY_PIN) | \
		WKUPCT_PIN_SELECT(CHG_DET_GPIO_PORT, CHG_DET_GPIO_PIN) \
	)

#define WAKEUP_KEY_POL_MASK \
	( \
		WKUPCT_PIN_POLARITY(WAKEUP_KEY_PORT, WAKEUP_KEY_PIN, KEY_ACTIVE_LOW) | \
		WKUPCT_PIN_POLARITY(CHG_DET_GPIO_PORT, CHG_DET_GPIO_PIN, CHG_DET_ACTIVE_LOW) \
	)

struct jwaoo_key_device
{
	uint8_t code;
	uint8_t value;
	uint8_t count;
	uint8_t repeat;
	uint8_t skip;
	uint8_t last_value;
	bool lock_enable;
	bool repeat_enable;
	bool long_click_enable;
	bool multi_click_enable;
	uint16_t repeat_timer;
	uint16_t long_click_timer;
	uint16_t multi_click_timer;
};

void jwaoo_key_init(void);
void jwaoo_key_repeat_fire(uint8_t key);
void jwaoo_key_long_click_fire(uint8_t key);
void jwaoo_key_multi_click_fire(uint8_t key);

