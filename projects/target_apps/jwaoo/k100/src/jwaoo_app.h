#pragma once

#include "gap.h"
#include "app.h"
#include "arch_api.h"
#include "jwaoo_hw.h"
#include "jwaoo_battery.h"

#define JWAOO_SUSPEND_DELAY_DEFAULT		600

#define APP_AD_MSD_COMPANY_ID			(0xABCD)
#define APP_AD_MSD_COMPANY_ID_LEN		(2)
#define APP_AD_MSD_DATA_LEN				(sizeof(uint16_t))

#define SEND_EMPTY_MESSAGE(msgid, taskid) \
	ke_msg_send(ke_msg_alloc(msgid, taskid, 0, 0))

enum {
	JWAOO_SET_ACTIVE = KE_FIRST_MSG(TASK_JWAOO_APP),
	JWAOO_SET_SUSPEND,
	JWAOO_SET_UPGRADE_ENABLE,
	JWAOO_SET_UPGRADE_DISABLE,
	JWAOO_SET_FACTORY_ENABLE,
	JWAOO_SET_FACTORY_DISABLE,

	JWAOO_ADV_START,
	JWAOO_REBOOT,
	JWAOO_SHUTDOWN,
	JWAOO_PROCESS_KEY,
	JWAOO_BT_LED_BLINK,
	JWAOO_MOTO_BOOST,
	JWAOO_MOTO_RAND_TIMER,

	JWAOO_SUSPEND_TIMER,
	JWAOO_KEY_LOCK_TIMER,
	JWAOO_BATT_POLL_TIMER,

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

#pragma pack(1)

typedef struct {
	uint16_t suspend_delay;
	uint16_t shutdown_voltage;
	uint16_t bt_led_close_time;
	uint8_t bt_led_open_time;
	uint8_t moto_rand_delay;
	uint8_t moto_rand_max;
	uint8_t moto_speed_min;
} jwaoo_app_settings_t;

typedef struct {
	bool click_enable;
	bool multi_click_enable;
	uint16_t multi_click_delay;
	bool long_click_enable;
	uint16_t long_click_delay;
} jwaoo_key_settings_t;
#pragma pack()

typedef struct {
	bool connected;
	bool disconnected;
	bool initialized;
	bool device_enabled;
	uint16_t suspend_counter;

	uint8_t moto_mode;
	uint8_t moto_speed;
	bool moto_report;
	bool moto_boost_busy;

	bool battery_report;
	uint8_t battery_skip;
	uint8_t battery_led_locked;
	uint8_t battery_state;
	uint8_t battery_level;
	uint8_t battery_full;
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
	bool key_app_locked;
	bool key_lock_pending;
	bool key_release_pending;

	uint8_t app_data[16];
} jwaoo_app_env_t;

extern jwaoo_app_env_t jwaoo_app_env;
extern jwaoo_app_settings_t jwaoo_app_settings;
extern jwaoo_key_settings_t jwaoo_key_settings;

ke_state_t jwaoo_app_state_get(void);
void jwaoo_app_state_set(ke_state_t const state_id);
void jwaoo_app_timer_clear(ke_msg_id_t const timer_id);
void jwaoo_app_timer_set(ke_msg_id_t const timer_id, uint32_t delay);
void jwaoo_app_msg_send(void const *param);

void jwaoo_app_init(void);
void jwaoo_app_adv_start(void);
void jwaoo_app_set_connect_state(bool connected);
void jwaoo_app_goto_active_mode(void);
void jwaoo_app_goto_suspend_mode(void);
void jwaoo_app_set_upgrade_enable(bool enable);
void jwaoo_app_set_factory_enable(bool enable);

void jwaoo_app_suspend_counter_reset(void);
void jwaoo_app_suspend_counter_start(void);

void jwaoo_app_before_sleep(void);
void jwaoo_app_resume_from_sleep(void);


static inline void *jwaoo_app_msg_alloc(ke_msg_id_t const id, uint16_t const param_len)
{
	return ke_msg_alloc(id, TASK_JWAOO_APP, 0, param_len);
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

static inline void jwaoo_battery_poll_start(void)
{
	jwaoo_app_timer_set(JWAOO_BATT_POLL_TIMER, 10);
}

static inline void jwaoo_battery_poll_stop(void)
{
	jwaoo_app_timer_clear(JWAOO_BATT_POLL_TIMER);
}

static inline void jwaoo_update_bt_led_state(void)
{
	jwaoo_app_timer_set(JWAOO_BT_LED_BLINK, 1);
}
