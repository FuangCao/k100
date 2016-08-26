#pragma once

#include "gap.h"
#include "app.h"
#include "arch_api.h"
#include "jwaoo_hw.h"

#define APP_AD_MSD_COMPANY_ID		(0xABCD)
#define APP_AD_MSD_COMPANY_ID_LEN	(2)
#define APP_AD_MSD_DATA_LEN			(sizeof(uint16_t))

enum {
	JWAOO_SET_ACTIVE = KE_FIRST_MSG(TASK_JWAOO_APP),
	JWAOO_SET_SUSPEND,
	JWAOO_SET_DEEP_SLEEP,

	JWAOO_ADV_START,
	JWAOO_ADV_STOP,
	JWAOO_BATT_POLL,
	JWAOO_REBOOT,
	JWAOO_SHUTDOWN,
	JWAOO_KEY_LOCK,
	JWAOO_MOTO_BOOST,
	JWAOO_MOTO_RAND,

	JWAOO_PWM1_BLINK_TIMER,
	JWAOO_PWM2_BLINK_TIMER,
	JWAOO_PWM3_BLINK_TIMER,

	JWAOO_KEY1_REPEAT_TIMER,
	JWAOO_KEY2_REPEAT_TIMER,
	JWAOO_KEY3_REPEAT_TIMER,
	JWAOO_KEY4_REPEAT_TIMER,

	JWAOO_KEY1_LONG_CLICK_TIMER,
	JWAOO_KEY2_LONG_CLICK_TIMER,
	JWAOO_KEY3_LONG_CLICK_TIMER,
	JWAOO_KEY4_LONG_CLICK_TIMER,

	JWAOO_KEY1_MULTI_CLICK_TIMER,
	JWAOO_KEY2_MULTI_CLICK_TIMER,
	JWAOO_KEY3_MULTI_CLICK_TIMER,
	JWAOO_KEY4_MULTI_CLICK_TIMER,
};

enum {
	JWAOO_APP_STATE_INVALID,
	JWAOO_APP_STATE_ACTIVE,
	JWAOO_APP_STATE_FACTORY,
	JWAOO_APP_STATE_SUSPEND,
	JWAOO_APP_STATE_DEEP_SLEEP,
	JWAOO_APP_STATE_MAX
};

struct jwaoo_app_data {
	bool initialized;
	bool device_enabled;
	bool deep_sleep_enabled;

	uint8_t moto_mode;
	bool moto_boost_busy;

	bool charge_online;
	uint8_t battery_led_locked;

	bool key_locked;
	bool key_lock_pending;
	bool key_click_enable;
	bool key_multi_click_enable;
	bool key_long_click_enable;
	uint16_t key_long_click_delay;
	uint16_t key_multi_click_delay;
};

extern struct jwaoo_app_data jwaoo_app_env;

ke_state_t jwaoo_app_state_get(void);
void jwaoo_app_state_set(ke_state_t const state_id);
bool jwaoo_app_timer_active(ke_msg_id_t const timer_id);
void jwaoo_app_timer_clear(ke_msg_id_t const timer_id);
void jwaoo_app_timer_set(ke_msg_id_t const timer_id, uint32_t delay);
void jwaoo_app_msg_send(void const *param);

void jwaoo_app_init(void);
void jwaoo_app_adv_start(void);
void jwaoo_app_adv_stop(void);
void jwaoo_app_goto_active_mode(void);
void jwaoo_app_goto_suspend_mode(void);
void jwaoo_app_goto_deep_sleep_mode(void);

void jwaoo_app_before_sleep(void);
void jwaoo_app_resume_from_sleep(void);
void jwaoo_app_update_suspend_timer(void);

static inline void *jwaoo_app_msg_alloc(ke_msg_id_t const id, ke_task_id_t const src_id, uint16_t const param_len)
{
	return ke_msg_alloc(id, TASK_JWAOO_APP, src_id, param_len);
}

static inline bool jwaoo_app_is_active(void)
{
	return jwaoo_app_state_get() == JWAOO_APP_STATE_ACTIVE;
}

static inline bool jwaoo_app_not_active(void)
{
	return jwaoo_app_state_get() != JWAOO_APP_STATE_ACTIVE;
}

static inline bool jwaoo_app_is_suspended(void)
{
	return jwaoo_app_state_get() == JWAOO_APP_STATE_SUSPEND;
}

static inline bool jwaoo_app_not_suspended(void)
{
	return jwaoo_app_state_get() != JWAOO_APP_STATE_SUSPEND;
}

static inline bool jwaoo_app_is_deep_sleep(void)
{
	return jwaoo_app_state_get() == JWAOO_APP_STATE_DEEP_SLEEP;
}

static inline bool jwaoo_app_not_deep_sleep(void)
{
	return jwaoo_app_state_get() != JWAOO_APP_STATE_DEEP_SLEEP;
}
