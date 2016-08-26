#include "jwaoo_app.h"
#include "jwaoo_key.h"
#include "jwaoo_pwm.h"
#include "jwaoo_moto.h"
#include "jwaoo_battery.h"

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

#if 1
	jwaoo_pwm_blink_square_full(JWAOO_PWM_BT_LED, 1000, 0);
#else
	jwaoo_pwm_blink_sawtooth_full(JWAOO_PWM_BT_LED, 5, 1000, 0);
#endif

	if (!ke_timer_active(JWAOO_BATT_POLL, dest_id)) {
		ke_timer_set(JWAOO_BATT_POLL, dest_id, 1);
	}

	return KE_MSG_CONSUMED;
}

static int jwaoo_adv_stop_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	app_easy_gap_advertise_stop();
	jwaoo_pwm_blink_close(JWAOO_PWM_BT_LED);

	return KE_MSG_CONSUMED;
}

static int jwaoo_battery_poll_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	ke_timer_set(msgid, dest_id, 100);

	return KE_MSG_CONSUMED;
}

static int jwaoo_reboot_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	return KE_MSG_CONSUMED;
}

static int jwaoo_shutdown_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	return KE_MSG_CONSUMED;
}

static int jwaoo_pwm_blink_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	jwaoo_pwm_blink_walk(msgid - JWAOO_PWM1_BLINK_TIMER);

	return KE_MSG_CONSUMED;
}

static int jwaoo_key_lock_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	jwaoo_key_lock_fire();

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
	jwaoo_moto_rand_fire();

	return KE_MSG_CONSUMED;
}

static int jwaoo_key_repeat_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	jwaoo_key_repeat_fire(msgid, msgid - JWAOO_KEY1_REPEAT_TIMER);

	return KE_MSG_CONSUMED;
}

static int jwaoo_key_long_click_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	jwaoo_key_long_click_fire(msgid, msgid - JWAOO_KEY1_LONG_CLICK_TIMER);

	return KE_MSG_CONSUMED;
}

static int jwaoo_key_multi_click_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	jwaoo_key_multi_click_fire(msgid, msgid - JWAOO_KEY1_MULTI_CLICK_TIMER);

	return KE_MSG_CONSUMED;
}

static int jwaoo_default_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	switch (msgid) {
	case JWAOO_PWM1_BLINK_TIMER:
	case JWAOO_PWM2_BLINK_TIMER:
	case JWAOO_PWM3_BLINK_TIMER:
		jwaoo_pwm_blink_close(msgid - JWAOO_PWM1_BLINK_TIMER);
		break;

	case JWAOO_MOTO_BOOST:
		jwaoo_app_env.moto_boost_busy = false;
		jwaoo_pwm_blink_close(JWAOO_PWM_MOTO);
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

static int jwaoo_active_to_deep_sleep_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	if (jwaoo_app_env.charge_online) {
		ke_state_set(TASK_JWAOO_APP, JWAOO_APP_STATE_SUSPEND);
		jwaoo_hw_set_suspend(true);
	} else {
		ke_state_set(TASK_JWAOO_APP, JWAOO_APP_STATE_DEEP_SLEEP);
		jwaoo_hw_set_suspend(true);
		jwaoo_hw_set_deep_sleep(true);
	}

	return KE_MSG_CONSUMED;
}

static int jwaoo_suspend_to_active_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	ke_state_set(TASK_JWAOO_APP, JWAOO_APP_STATE_ACTIVE);

	jwaoo_hw_set_suspend(false);

	return KE_MSG_CONSUMED;
}

static int jwaoo_suspend_to_deep_sleep_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	if (!jwaoo_app_env.charge_online) {
		ke_state_set(TASK_JWAOO_APP, JWAOO_APP_STATE_DEEP_SLEEP);
		jwaoo_hw_set_deep_sleep(true);
	}

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

static const struct ke_msg_handler jwaoo_app_active_handlers[] = {
	{ KE_MSG_DEFAULT_HANDLER,					(ke_msg_func_t) jwaoo_default_handler },
	{ JWAOO_SET_SUSPEND, 						(ke_msg_func_t) jwaoo_active_to_suspend_handler },
	{ JWAOO_SET_DEEP_SLEEP, 					(ke_msg_func_t) jwaoo_active_to_deep_sleep_handler },
	{ JWAOO_ADV_START,							(ke_msg_func_t) jwaoo_adv_start_handler },
	{ JWAOO_ADV_STOP,							(ke_msg_func_t) jwaoo_adv_stop_handler },
	{ JWAOO_BATT_POLL,							(ke_msg_func_t) jwaoo_battery_poll_handler },
	{ JWAOO_REBOOT,								(ke_msg_func_t) jwaoo_reboot_handler },
	{ JWAOO_SHUTDOWN,							(ke_msg_func_t) jwaoo_shutdown_handler },
	{ JWAOO_MOTO_BOOST,							(ke_msg_func_t) jwaoo_moto_boost_handler },
	{ JWAOO_MOTO_RAND,							(ke_msg_func_t) jwaoo_moto_rand_handler },
	{ JWAOO_KEY_LOCK,							(ke_msg_func_t) jwaoo_key_lock_handler },

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

static const struct ke_msg_handler jwaoo_app_suspend_handlers[] = {
	{ KE_MSG_DEFAULT_HANDLER,					(ke_msg_func_t) jwaoo_default_handler },
	{ JWAOO_SET_ACTIVE, 						(ke_msg_func_t) jwaoo_suspend_to_active_handler },
	{ JWAOO_SET_DEEP_SLEEP,						(ke_msg_func_t) jwaoo_suspend_to_deep_sleep_handler },
	{ JWAOO_ADV_STOP,							(ke_msg_func_t) jwaoo_adv_stop_handler },
	{ JWAOO_BATT_POLL,							(ke_msg_func_t) jwaoo_battery_poll_handler },
	{ JWAOO_PWM_TIMER(JWAOO_PWM_BATT_LED),		(ke_msg_func_t) jwaoo_pwm_blink_handler}
};

static const struct ke_msg_handler jwaoo_app_deep_sleep_handlers[] = {
	{ KE_MSG_DEFAULT_HANDLER,					(ke_msg_func_t) jwaoo_default_handler },
	{ JWAOO_SET_ACTIVE, 						(ke_msg_func_t) jwaoo_deep_sleep_to_active_handler },
	{ JWAOO_SET_SUSPEND, 						(ke_msg_func_t) jwaoo_deep_sleep_to_suspend_handler },
	{ JWAOO_ADV_STOP,							(ke_msg_func_t) jwaoo_adv_stop_handler },
};

static const struct ke_state_handler jwaoo_app_state_handler[JWAOO_APP_STATE_MAX] = {
    [JWAOO_APP_STATE_ACTIVE]					= KE_STATE_HANDLER(jwaoo_app_active_handlers),
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

bool jwaoo_app_timer_active(ke_msg_id_t const timer_id)
{
	if (jwaoo_app_env.initialized) {
		return ke_timer_active(timer_id, TASK_JWAOO_APP);
	}

	return false;
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
	jwaoo_app_mnf_data_init();

	ke_task_create(TASK_JWAOO_APP, &TASK_DESC_JWAOO_APP);
	ke_state_set(TASK_JWAOO_APP, JWAOO_APP_STATE_ACTIVE);

	jwaoo_app_env.initialized = true;

	jwaoo_hw_set_deep_sleep(false);
	jwaoo_hw_set_suspend(false);
}

static void jwaoo_app_set_mode(ke_msg_id_t const msgid)
{
	jwaoo_app_msg_send(jwaoo_app_msg_alloc(msgid, 0, 0));
}

void jwaoo_app_goto_active_mode(void)
{
	jwaoo_app_set_mode(JWAOO_SET_ACTIVE);
}

void jwaoo_app_goto_suspend_mode(void)
{
	jwaoo_app_set_mode(JWAOO_SET_SUSPEND);
}

void jwaoo_app_goto_deep_sleep_mode(void)
{
	jwaoo_app_set_mode(JWAOO_SET_DEEP_SLEEP);
}

void jwaoo_app_adv_start(void)
{
	jwaoo_app_msg_send(jwaoo_app_msg_alloc(JWAOO_ADV_START, 0, 0));
}

void jwaoo_app_adv_stop(void)
{
	jwaoo_app_msg_send(jwaoo_app_msg_alloc(JWAOO_ADV_STOP, 0, 0));
}

// ================================================================================

void jwaoo_app_before_sleep(void)
{
	rwip_env.ext_wakeup_enable = 2;
	wkupct_enable_irq(WAKEUP_KEY_SEL_MASK, WAKEUP_KEY_POL_MASK, 1, 60);
}

void jwaoo_app_resume_from_sleep(void)
{
	wkupct_disable_irq();

	jwaoo_hw_set_deep_sleep(false);

	jwaoo_app_env.charge_online = CHG_ONLINE;

	if (WAKEUP_KEY_ACTIVE) {
		jwaoo_app_goto_active_mode();
	} else if (jwaoo_app_env.charge_online) {
		jwaoo_app_goto_suspend_mode();
	} else {
		jwaoo_hw_set_deep_sleep(true);
	}
}

void jwaoo_app_update_suspend_timer(void)
{
}
