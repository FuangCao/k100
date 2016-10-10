#include "jwaoo_app.h"
#include "jwaoo_key.h"
#include "jwaoo_pwm.h"
#include "jwaoo_spi.h"
#include "jwaoo_moto.h"

struct mnf_specific_data_ad_structure
{
    uint8_t ad_structure_size;
    uint8_t ad_structure_type;
    uint8_t company_id[APP_AD_MSD_COMPANY_ID_LEN];
    uint8_t proprietary_data[APP_AD_MSD_DATA_LEN];
};

struct jwaoo_app_data jwaoo_app_env __attribute__((section("retention_mem_area0"), zero_init)); //@RETENTION MEMORY
static struct mnf_specific_data_ad_structure mnf_data __attribute__((section("retention_mem_area0"), zero_init)); //@RETENTION MEMORY

static void jwaoo_app_mnf_data_init(void)
{
	mnf_data.ad_structure_size = sizeof(struct mnf_specific_data_ad_structure ) - sizeof(uint8_t); // minus the size of the ad_structure_size field
	mnf_data.ad_structure_type = GAP_AD_TYPE_MANU_SPECIFIC_DATA;
	mnf_data.company_id[0] = APP_AD_MSD_COMPANY_ID & 0xFF; // LSB
	mnf_data.company_id[1] = (APP_AD_MSD_COMPANY_ID >> 8 )& 0xFF; // MSB
	mnf_data.proprietary_data[0] = 0;
	mnf_data.proprietary_data[1] = 0;
}

static void jwaoo_app_mnf_data_update(void)
{
	uint16_t data;

	data = mnf_data.proprietary_data[0] | (mnf_data.proprietary_data[1] << 8);
	data += 1;
	mnf_data.proprietary_data[0] = data & 0xFF;
	mnf_data.proprietary_data[1] = (data >> 8) & 0xFF;

	if (data == 0xFFFF) {
		mnf_data.proprietary_data[0] = 0;
		mnf_data.proprietary_data[1] = 0;
	}
}

static void jwaoo_app_add_ad_struct(struct gapm_start_advertise_cmd *cmd, void *ad_struct_data, uint8_t ad_struct_len)
{
    if ( (APP_ADV_DATA_MAX_SIZE - cmd->info.host.adv_data_len) >= ad_struct_len) {
        memcpy(&cmd->info.host.adv_data[cmd->info.host.adv_data_len], ad_struct_data, ad_struct_len);
        cmd->info.host.adv_data_len += ad_struct_len;
    } else if ( (APP_SCAN_RESP_DATA_MAX_SIZE - cmd->info.host.scan_rsp_data_len) >= ad_struct_len) {
        memcpy(&cmd->info.host.scan_rsp_data[cmd->info.host.scan_rsp_data_len], ad_struct_data, ad_struct_len);
        cmd->info.host.scan_rsp_data_len += ad_struct_len;
    }
}

static int jwaoo_adv_start_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	struct gapm_start_advertise_cmd *cmd = app_easy_gap_undirected_advertise_get_active();

	jwaoo_app_mnf_data_update();
	jwaoo_app_add_ad_struct(cmd, &mnf_data, sizeof(struct mnf_specific_data_ad_structure));

	app_easy_gap_undirected_advertise_start();
	jwaoo_pwm_blink_square_full(JWAOO_PWM_BT_LED, 1000, 0);
	jwaoo_battery_poll_start();

	jwaoo_app_suspend_counter_start();

	return KE_MSG_CONSUMED;
}

static int jwaoo_active_battery_poll_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	jwaoo_battery_poll();

	return KE_MSG_CONSUMED;
}

static int jwaoo_suspend_battery_poll_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	if (jwaoo_app_env.charge_online) {
		jwaoo_battery_poll();
	} else if (jwaoo_key_get_press_count()) {
		jwaoo_app_timer_set(JWAOO_BATT_POLL_TIMER, 20);
		jwaoo_pwm_blink_close(JWAOO_PWM_BATT_LED);
	} else {
		ke_state_set(TASK_JWAOO_APP, JWAOO_APP_STATE_DEEP_SLEEP);
		jwaoo_hw_set_suspend(true);
		jwaoo_hw_set_deep_sleep(true);
	}

	return KE_MSG_CONSUMED;
}

static int jwaoo_reboot_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	platform_reset(RESET_AFTER_SPOTA_UPDATE);

	return KE_MSG_CONSUMED;
}

static int jwaoo_shutdown_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	jwaoo_app_goto_suspend_mode();

	return KE_MSG_CONSUMED;
}

static int jwaoo_pwm_blink_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	jwaoo_pwm_blink_walk(msgid - JWAOO_PWM1_BLINK_TIMER);

	return KE_MSG_CONSUMED;
}

static int jwaoo_key_lock_timer_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	jwaoo_key_lock_timer_fire();

	return KE_MSG_CONSUMED;
}

static int jwaoo_suspend_timer_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	if (jwaoo_app_env.connected || jwaoo_moto_get_speed()) {
		jwaoo_app_suspend_counter_reset();
	} else if (jwaoo_app_env.key_locked) {
		jwaoo_app_goto_suspend_mode();
	} else {
		jwaoo_app_timer_set(msgid, 100);

		if (jwaoo_app_env.suspend_counter > 0) {
			jwaoo_app_env.suspend_counter--;
		} else {
			jwaoo_app_goto_suspend_mode();
		}
	}

	return KE_MSG_CONSUMED;
}

static int jwaoo_moto_boost_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	jwaoo_app_env.moto_boost_busy = false;
	jwaoo_pwm_sync(JWAOO_PWM_MOTO);

	return KE_MSG_CONSUMED;
}

static int jwaoo_moto_rand_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	jwaoo_moto_rand_timer_fire();

	return KE_MSG_CONSUMED;
}

static int jwaoo_key_repeat_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	jwaoo_key_repeat_timer_fire(msgid, msgid - JWAOO_KEY1_REPEAT_TIMER);

	return KE_MSG_CONSUMED;
}

static int jwaoo_key_long_click_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	jwaoo_key_long_click_timer_fire(msgid, msgid - JWAOO_KEY1_LONG_CLICK_TIMER);

	return KE_MSG_CONSUMED;
}

static int jwaoo_key_multi_click_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	jwaoo_key_multi_click_timer_fire(msgid, msgid - JWAOO_KEY1_MULTI_CLICK_TIMER);

	return KE_MSG_CONSUMED;
}

static int jwaoo_default_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	switch (msgid) {
	case JWAOO_PWM1_BLINK_TIMER:
	case JWAOO_PWM2_BLINK_TIMER:
	case JWAOO_PWM3_BLINK_TIMER: {
			uint8_t pwm = msgid - JWAOO_PWM1_BLINK_TIMER;

			jwaoo_pwm_blink_close(pwm);
			jwaoo_pwm_set_complete(pwm);
		}
		break;

	case JWAOO_MOTO_BOOST:
		jwaoo_app_env.moto_boost_busy = false;
		jwaoo_pwm_blink_close(JWAOO_PWM_MOTO);
		break;

	case JWAOO_SUSPEND_TIMER:
		jwaoo_app_suspend_counter_reset();
		break;
	}

	return KE_MSG_CONSUMED;
}

// ================================================================================

static int jwaoo_active_to_suspend_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	ke_state_set(TASK_JWAOO_APP, JWAOO_APP_STATE_SUSPEND);

	jwaoo_hw_set_suspend(true);

	return KE_MSG_CONSUMED;
}

static int jwaoo_suspend_to_active_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	ke_state_set(TASK_JWAOO_APP, JWAOO_APP_STATE_ACTIVE);

	jwaoo_hw_set_suspend(false);

	return KE_MSG_CONSUMED;
}

static int jwaoo_deep_sleep_to_suspend_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	ke_state_set(TASK_JWAOO_APP, JWAOO_APP_STATE_SUSPEND);

	// jwaoo_hw_set_deep_sleep(false);

	return KE_MSG_CONSUMED;
}

static int jwaoo_deep_sleep_to_active_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	ke_state_set(TASK_JWAOO_APP, JWAOO_APP_STATE_ACTIVE);

	// jwaoo_hw_set_deep_sleep(false);
	jwaoo_hw_set_suspend(false);

	return KE_MSG_CONSUMED;
}

static int jwaoo_set_upgrade_enable_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	ke_state_set(TASK_JWAOO_APP, JWAOO_APP_STATE_UPGRADE);

	return KE_MSG_CONSUMED;
}

static int jwaoo_set_upgrade_disable_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	ke_state_set(TASK_JWAOO_APP, JWAOO_APP_STATE_ACTIVE);

	jwaoo_battery_poll_start();

	return KE_MSG_CONSUMED;
}

static int jwaoo_set_factory_enable_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	ke_state_set(TASK_JWAOO_APP, JWAOO_APP_STATE_FACTORY);

	jwaoo_app_env.battery_led_locked = 3;
	jwaoo_pwm_blink_close(JWAOO_PWM_BATT_LED);

	return KE_MSG_CONSUMED;
}

static int jwaoo_set_factory_disable_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	ke_state_set(TASK_JWAOO_APP, JWAOO_APP_STATE_ACTIVE);

	jwaoo_battery_led_release(3);

	return KE_MSG_CONSUMED;
}

static int jwaoo_active_process_key_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	jwaoo_key_process_active(*(const uint8_t *) param);

	return KE_MSG_CONSUMED;
}

static int jwaoo_suspend_process_key_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	jwaoo_key_process_suspend(*(const uint8_t *) param);

	return KE_MSG_CONSUMED;
}

static int jwaoo_factory_process_key_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	jwaoo_key_process_factory(*(const uint8_t *) param);

	return KE_MSG_CONSUMED;
}

static const struct ke_msg_handler jwaoo_app_active_handlers[] = {
	{ KE_MSG_DEFAULT_HANDLER,					(ke_msg_func_t) jwaoo_default_handler },
	{ JWAOO_SET_SUSPEND, 						(ke_msg_func_t) jwaoo_active_to_suspend_handler },
	{ JWAOO_SET_UPGRADE_ENABLE, 				(ke_msg_func_t) jwaoo_set_upgrade_enable_handler },
	{ JWAOO_SET_FACTORY_ENABLE, 				(ke_msg_func_t) jwaoo_set_factory_enable_handler },

	{ JWAOO_ADV_START,							(ke_msg_func_t) jwaoo_adv_start_handler },
	{ JWAOO_REBOOT,								(ke_msg_func_t) jwaoo_reboot_handler },
	{ JWAOO_SHUTDOWN,							(ke_msg_func_t) jwaoo_shutdown_handler },
	{ JWAOO_PROCESS_KEY,						(ke_msg_func_t) jwaoo_active_process_key_handler },
	{ JWAOO_MOTO_BOOST,							(ke_msg_func_t) jwaoo_moto_boost_handler },
	{ JWAOO_MOTO_RAND_TIMER,					(ke_msg_func_t) jwaoo_moto_rand_handler },

	{ JWAOO_SUSPEND_TIMER,						(ke_msg_func_t) jwaoo_suspend_timer_handler },
	{ JWAOO_KEY_LOCK_TIMER,						(ke_msg_func_t) jwaoo_key_lock_timer_handler },
	{ JWAOO_BATT_POLL_TIMER,					(ke_msg_func_t) jwaoo_active_battery_poll_handler },

	{ JWAOO_PWM1_BLINK_TIMER,					(ke_msg_func_t) jwaoo_pwm_blink_handler },
	{ JWAOO_PWM2_BLINK_TIMER,					(ke_msg_func_t) jwaoo_pwm_blink_handler },
	{ JWAOO_PWM3_BLINK_TIMER,					(ke_msg_func_t) jwaoo_pwm_blink_handler },

	{ JWAOO_KEY1_REPEAT_TIMER,					(ke_msg_func_t) jwaoo_key_repeat_handler },
	{ JWAOO_KEY2_REPEAT_TIMER,					(ke_msg_func_t) jwaoo_key_repeat_handler },
	{ JWAOO_KEY3_REPEAT_TIMER,					(ke_msg_func_t) jwaoo_key_repeat_handler },
	{ JWAOO_KEY4_REPEAT_TIMER,					(ke_msg_func_t) jwaoo_key_repeat_handler },

	{ JWAOO_KEY1_LONG_CLICK_TIMER,				(ke_msg_func_t) jwaoo_key_long_click_handler },
	{ JWAOO_KEY2_LONG_CLICK_TIMER,				(ke_msg_func_t) jwaoo_key_long_click_handler },
	{ JWAOO_KEY3_LONG_CLICK_TIMER,				(ke_msg_func_t) jwaoo_key_long_click_handler },
	{ JWAOO_KEY4_LONG_CLICK_TIMER,				(ke_msg_func_t) jwaoo_key_long_click_handler },

	{ JWAOO_KEY1_MULTI_CLICK_TIMER,				(ke_msg_func_t) jwaoo_key_multi_click_handler },
	{ JWAOO_KEY2_MULTI_CLICK_TIMER,				(ke_msg_func_t) jwaoo_key_multi_click_handler },
	{ JWAOO_KEY3_MULTI_CLICK_TIMER,				(ke_msg_func_t) jwaoo_key_multi_click_handler },
	{ JWAOO_KEY4_MULTI_CLICK_TIMER,				(ke_msg_func_t) jwaoo_key_multi_click_handler },
};

static const struct ke_msg_handler jwaoo_app_upgrade_handlers[] = {
	{ KE_MSG_DEFAULT_HANDLER,					(ke_msg_func_t) jwaoo_default_handler },
	{ JWAOO_SET_UPGRADE_DISABLE,				(ke_msg_func_t) jwaoo_set_upgrade_disable_handler },
	{ JWAOO_ADV_START,							(ke_msg_func_t) jwaoo_adv_start_handler },
};

static const struct ke_msg_handler jwaoo_app_factory_handlers[] = {
	{ KE_MSG_DEFAULT_HANDLER,					(ke_msg_func_t) jwaoo_default_handler },
	{ JWAOO_SET_FACTORY_DISABLE,				(ke_msg_func_t) jwaoo_set_factory_disable_handler },
	{ JWAOO_ADV_START,							(ke_msg_func_t) jwaoo_adv_start_handler },
	{ JWAOO_BATT_POLL_TIMER,					(ke_msg_func_t) jwaoo_active_battery_poll_handler },
	{ JWAOO_MOTO_BOOST,							(ke_msg_func_t) jwaoo_moto_boost_handler },
	{ JWAOO_PROCESS_KEY,						(ke_msg_func_t) jwaoo_factory_process_key_handler },
};

static const struct ke_msg_handler jwaoo_app_suspend_handlers[] = {
	{ KE_MSG_DEFAULT_HANDLER,					(ke_msg_func_t) jwaoo_default_handler },
	{ JWAOO_SET_ACTIVE, 						(ke_msg_func_t) jwaoo_suspend_to_active_handler },
	{ JWAOO_BATT_POLL_TIMER,					(ke_msg_func_t) jwaoo_suspend_battery_poll_handler },
	{ JWAOO_PWM_TIMER(JWAOO_PWM_BATT_LED),		(ke_msg_func_t) jwaoo_pwm_blink_handler },
	{ JWAOO_PROCESS_KEY,						(ke_msg_func_t) jwaoo_suspend_process_key_handler },
};

static const struct ke_msg_handler jwaoo_app_deep_sleep_handlers[] = {
	{ KE_MSG_DEFAULT_HANDLER,					(ke_msg_func_t) jwaoo_default_handler },
	{ JWAOO_SET_ACTIVE, 						(ke_msg_func_t) jwaoo_deep_sleep_to_active_handler },
	{ JWAOO_SET_SUSPEND, 						(ke_msg_func_t) jwaoo_deep_sleep_to_suspend_handler },
};

static const struct ke_state_handler jwaoo_app_state_handler[JWAOO_APP_STATE_MAX] = {
	[JWAOO_APP_STATE_ACTIVE]					= KE_STATE_HANDLER(jwaoo_app_active_handlers),
	[JWAOO_APP_STATE_UPGRADE]					= KE_STATE_HANDLER(jwaoo_app_upgrade_handlers),
	[JWAOO_APP_STATE_FACTORY]					= KE_STATE_HANDLER(jwaoo_app_factory_handlers),
	[JWAOO_APP_STATE_SUSPEND]					= KE_STATE_HANDLER(jwaoo_app_suspend_handlers),
	[JWAOO_APP_STATE_DEEP_SLEEP] 				= KE_STATE_HANDLER(jwaoo_app_deep_sleep_handlers),
};

static ke_state_t __attribute__((section("retention_mem_area0"), zero_init)) jwaoo_app_state;

static const struct ke_state_handler jwaoo_app_default_handler = KE_STATE_HANDLER(jwaoo_app_active_handlers);
static const struct ke_task_desc TASK_DESC_JWAOO_APP = { jwaoo_app_state_handler, &jwaoo_app_default_handler, &jwaoo_app_state, JWAOO_APP_STATE_MAX, 1};

ke_state_t jwaoo_app_state_get(void)
{
	if (jwaoo_app_env.initialized) {
		return ke_state_get(TASK_JWAOO_APP);
	} else {
		return JWAOO_APP_STATE_INVALID;
	}
}

void jwaoo_app_state_set(ke_state_t const state_id)
{
	if (jwaoo_app_env.initialized) {
		ke_state_set(TASK_JWAOO_APP, state_id);
	}
}

void jwaoo_app_timer_clear(ke_msg_id_t const timer_id)
{
	if (jwaoo_app_env.initialized) {
		ke_timer_clear(timer_id, TASK_JWAOO_APP);
	}
}

void jwaoo_app_timer_set(ke_msg_id_t const timer_id, uint32_t delay)
{
	if (jwaoo_app_env.initialized) {
		ke_timer_set(timer_id, TASK_JWAOO_APP, delay);
	}
}

void jwaoo_app_msg_send(void const *param)
{
	if (jwaoo_app_env.initialized) {
		ke_msg_send(param);
	} else {
		ke_msg_free(ke_param2msg(param));
	}
}

void jwaoo_app_init(void)
{
	jwaoo_app_env.sensor_poll_delay = 20;
	jwaoo_app_env.key_long_click_delay = 200;
	jwaoo_app_env.key_multi_click_delay = 30;

	jwaoo_app_mnf_data_init();

	ke_task_create(TASK_JWAOO_APP, &TASK_DESC_JWAOO_APP);
	ke_state_set(TASK_JWAOO_APP, JWAOO_APP_STATE_ACTIVE);

	jwaoo_hw_set_deep_sleep(false);

	jwaoo_app_env.initialized = true;
}

static void jwaoo_app_set_mode(ke_msg_id_t const msgid)
{
	jwaoo_app_msg_send(jwaoo_app_msg_alloc(msgid, 0));
}

void jwaoo_app_goto_active_mode(void)
{
	if (!jwaoo_app_env.key_locked) {
		jwaoo_app_set_mode(JWAOO_SET_ACTIVE);
	}
}

void jwaoo_app_goto_suspend_mode(void)
{
	if ((GetWord16(SYS_STAT_REG) & DBG_IS_UP) == 0) {
		jwaoo_app_set_mode(JWAOO_SET_SUSPEND);
	}
}

void jwaoo_app_set_upgrade_enable(bool enable)
{
	if (enable) {
		jwaoo_app_set_mode(JWAOO_SET_UPGRADE_ENABLE);
	} else {
		jwaoo_app_set_mode(JWAOO_SET_UPGRADE_DISABLE);
	}
}

void jwaoo_app_set_factory_enable(bool enable)
{
	if (enable) {
		jwaoo_app_set_mode(JWAOO_SET_FACTORY_ENABLE);
	} else {
		jwaoo_app_set_mode(JWAOO_SET_FACTORY_DISABLE);
	}
}

void jwaoo_app_adv_start(void)
{
	jwaoo_app_msg_send(jwaoo_app_msg_alloc(JWAOO_ADV_START, 0));
}

void jwaoo_app_set_connect_state(bool connected)
{
	jwaoo_app_env.connected = connected;

	if (connected) {
		jwaoo_pwm_blink_open(JWAOO_PWM_BT_LED);
	} else {
		jwaoo_pwm_blink_close(JWAOO_PWM_BT_LED);
	}
}

void jwaoo_app_suspend_counter_reset(void)
{
	jwaoo_app_env.suspend_counter = jwaoo_user_data.suspend_delay;
}

void jwaoo_app_suspend_counter_start(void)
{
	jwaoo_app_suspend_counter_reset();
	jwaoo_app_timer_set(JWAOO_SUSPEND_TIMER, 100);
}

// ================================================================================

void jwaoo_app_before_sleep(void)
{
	uint32_t pin_mask, pol_mask;

	rwip_env.ext_wakeup_enable = 2;

	WAKEUP_KEY_MASK_BUILD_ALL(pin_mask, pol_mask);
	wkupct_enable_irq(pin_mask, pol_mask, 1, 60);
}

static bool jwaoo_app_need_wakeup(void)
{
	if (jwaoo_app_env.key_locked) {
		uint32_t count = 0;

		while (WAKEUP_KEY1_ACTIVE) {
			if (WAKEUP_KEY2_ACTIVE) {
				if (++count > 500000) {
					return true;
				}
			} else {
				count = 0;
			}
		}
	} else if (WAKEUP_KEY1_ACTIVE) {
		return true;
	}

	return false;
}

void jwaoo_app_resume_from_sleep(void)
{
#if USE_WDOG
	wdg_freeze();
#endif

	jwaoo_app_env.key_lock_pending = false;
	jwaoo_app_env.key_release_pending = true;

	wkupct_disable_irq();
	jwaoo_hw_set_deep_sleep(false);

	if (jwaoo_app_need_wakeup()) {
		jwaoo_app_env.key_locked = false;
		jwaoo_app_goto_active_mode();
	} else if (jwaoo_app_env.charge_online) {
		jwaoo_app_goto_suspend_mode();
	} else {
		jwaoo_hw_set_deep_sleep(true);
	}

#if USE_WDOG
	wdg_reload(WATCHDOG_DEFAULT_PERIOD);
	wdg_resume();
#endif
}
