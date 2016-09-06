#pragma once

#include "gap.h"
#include "app.h"
#include "arch_api.h"
#include "jwaoo_hw.h"
#include "jwaoo_battery.h"

#define APP_AD_MSD_COMPANY_ID		(0xABCD)
#define APP_AD_MSD_COMPANY_ID_LEN	(2)
#define APP_AD_MSD_DATA_LEN			(sizeof(uint16_t))

enum {
	JWAOO_SET_ACTIVE = KE_FIRST_MSG(TASK_JWAOO_APP),
	JWAOO_SET_SUSPEND,
	JWAOO_SET_DEEP_SLEEP,
	JWAOO_SET_UPGRADE_ENABLE,
	JWAOO_SET_UPGRADE_DISABLE,
	JWAOO_SET_FACTORY_ENABLE,
	JWAOO_SET_FACTORY_DISABLE,

	JWAOO_ADV_START,
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
	JWAOO_APP_STATE_UPGRADE,
	JWAOO_APP_STATE_FACTORY,
	JWAOO_APP_STATE_SUSPEND,
	JWAOO_APP_STATE_DEEP_SLEEP,
	JWAOO_APP_STATE_MAX
};

struct jwaoo_app_data {
	bool initialized;
	bool device_enabled;

	uint8_t moto_mode;
	bool moto_boost_busy;

	bool charge_online;
	bool battery_report;
	uint8_t battery_led_locked;
	uint8_t battery_state;
	uint8_t battery_level;
	uint16_t battery_voltage;
	uint8_t battery_voltage_head;
	uint8_t battery_voltage_count;
	uint16_t battery_voltages[JWAOO_VOLTAGE_ARRAY_SIZE];

	bool flash_write_enable;
	bool flash_write_success;
	uint8_t flash_write_crc;
	uint16_t flash_write_length;
	uint32_t flash_write_offset;

	bool sensor_enable;
	bool sensor_pending;
	bool sensor_poll_enable;
	uint8_t sensor_accel_dead;
	uint8_t sensor_capacity_dead;
	uint16_t sensor_poll_delay;

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
void jwaoo_app_goto_active_mode(void);
void jwaoo_app_goto_suspend_mode(void);
void jwaoo_app_goto_deep_sleep_mode(void);
void jwaoo_app_set_upgrade_enable(bool enable);
void jwaoo_app_set_factory_enable(bool enable);

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

static inline bool jwaoo_app_is_upgrade(void)
{
	return jwaoo_app_state_get() == JWAOO_APP_STATE_UPGRADE;
}

static inline bool jwaoo_app_not_upgrade(void)
{
	return jwaoo_app_state_get() != JWAOO_APP_STATE_UPGRADE;
}

static inline bool jwaoo_app_is_factory(void)
{
	return jwaoo_app_state_get() == JWAOO_APP_STATE_FACTORY;
}

static inline bool jwaoo_app_not_factory(void)
{
	return jwaoo_app_state_get() != JWAOO_APP_STATE_FACTORY;
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
