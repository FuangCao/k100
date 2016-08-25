#pragma once

#include "gap.h"
#include "app.h"
#include "arch_api.h"
#include "jwaoo_hw.h"

#define APP_AD_MSD_COMPANY_ID		(0xABCD)
#define APP_AD_MSD_COMPANY_ID_LEN	(2)
#define APP_AD_MSD_DATA_LEN			(sizeof(uint16_t))

enum
{
	JWAOO_SET_ACTIVE = KE_FIRST_MSG(TASK_JWAOO_APP),
	JWAOO_SET_SUSPEND,
	JWAOO_SET_DEEP_SLEEP,

	JWAOO_ADV_START,
	JWAOO_BATT_POLL,
	JWAOO_REBOOT,
	JWAOO_SHUTDOWN,
	JWAOO_KEY_LOCK,

	JWAOO_LED1_BLINK,
	JWAOO_LED2_BLINK,
	JWAOO_MOTO_BLINK,
	JWAOO_MOTO_BOOST,

	JWAOO_KEY1_REPEAT_TIMER,
	JWAOO_KEY1_LONG_CLICK_TIMER,
	JWAOO_KEY1_MULTI_CLICK_TIMER,

	JWAOO_KEY2_REPEAT_TIMER,
	JWAOO_KEY2_LONG_CLICK_TIMER,
	JWAOO_KEY2_MULTI_CLICK_TIMER,

	JWAOO_KEY3_REPEAT_TIMER,
	JWAOO_KEY3_LONG_CLICK_TIMER,
	JWAOO_KEY3_MULTI_CLICK_TIMER,

	JWAOO_KEY4_REPEAT_TIMER,
	JWAOO_KEY4_LONG_CLICK_TIMER,
	JWAOO_KEY4_MULTI_CLICK_TIMER,
};

enum
{
	JWAOO_APP_STATE_ACTIVE,
	JWAOO_APP_STATE_SUSPEND,
	JWAOO_APP_STATE_DEEP_SLEEP,
	JWAOO_APP_STATE_MAX
};

void jwaoo_app_init(void);
void jwaoo_app_adv_start(void);
void jwaoo_app_goto_active_mode(void);
void jwaoo_app_goto_suspend_mode(void);
void jwaoo_app_goto_deep_sleep_mode(void);

void jwaoo_app_before_sleep(void);
void jwaoo_app_resume_from_sleep(void);
enum arch_main_loop_callback_ret jwaoo_app_ble_powered(void);

static inline ke_state_t jwaoo_app_get_state(void)
{
	return ke_state_get(TASK_JWAOO_APP);
}

static inline bool jwaoo_app_is_active(void)
{
	return jwaoo_app_get_state() == JWAOO_APP_STATE_ACTIVE;
}

static inline bool jwaoo_app_not_active(void)
{
	return jwaoo_app_get_state() != JWAOO_APP_STATE_ACTIVE;
}

static inline bool jwaoo_app_is_suspended(void)
{
	return jwaoo_app_get_state() == JWAOO_APP_STATE_SUSPEND;
}

static inline bool jwaoo_app_not_suspended(void)
{
	return jwaoo_app_get_state() != JWAOO_APP_STATE_SUSPEND;
}

static inline bool jwaoo_app_is_deep_sleep(void)
{
	return jwaoo_app_get_state() == JWAOO_APP_STATE_DEEP_SLEEP;
}

static inline bool jwaoo_app_not_deep_sleep(void)
{
	return jwaoo_app_get_state() != JWAOO_APP_STATE_DEEP_SLEEP;
}
