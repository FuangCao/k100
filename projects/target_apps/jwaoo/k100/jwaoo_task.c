#include "jwaoo_task.h"
#include "user_k100.h"

bool jwaoo_suspended;
bool jwaoo_deep_sleep;

static int jwaoo_battery_poll_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
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

static int jwaoo_led1_blink_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	return KE_MSG_CONSUMED;
}

static int jwaoo_led2_blink_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	return KE_MSG_CONSUMED;
}

static int jwaoo_moto_blink_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	return KE_MSG_CONSUMED;
}

static int jwaoo_moto_boost_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	return KE_MSG_CONSUMED;
}

static int jwaoo_key_lock_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	return KE_MSG_CONSUMED;
}

static int jwaoo_key_repeat_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	return KE_MSG_CONSUMED;
}

static int jwaoo_key_long_click_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	return KE_MSG_CONSUMED;
}

static int jwaoo_key_multi_click_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	return KE_MSG_CONSUMED;
}

static int jwaoo_dummy_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	return KE_MSG_CONSUMED;
}

static const struct ke_msg_handler jwaoo_app_active_handlers[] = {
	{ JWAOO_BATT_POLL,					(ke_msg_func_t) jwaoo_battery_poll_handler },
	{ JWAOO_REBOOT,						(ke_msg_func_t) jwaoo_reboot_handler },
	{ JWAOO_SHUTDOWN,					(ke_msg_func_t) jwaoo_shutdown_handler },
	{ JWAOO_LED1_BLINK,					(ke_msg_func_t) jwaoo_led1_blink_handler },
	{ JWAOO_LED2_BLINK,					(ke_msg_func_t) jwaoo_led2_blink_handler },
	{ JWAOO_MOTO_BLINK,					(ke_msg_func_t) jwaoo_moto_blink_handler },
	{ JWAOO_MOTO_BOOST,					(ke_msg_func_t) jwaoo_moto_boost_handler },
	{ JWAOO_KEY_LOCK,					(ke_msg_func_t) jwaoo_key_lock_handler },
	{ JWAOO_KEY1_REPEAT_TIMER,			(ke_msg_func_t) jwaoo_key_repeat_handler },
	{ JWAOO_KEY1_LONG_CLICK_TIMER,		(ke_msg_func_t) jwaoo_key_long_click_handler },
	{ JWAOO_KEY1_MULTI_CLICK_TIMER,		(ke_msg_func_t) jwaoo_key_multi_click_handler },
	{ JWAOO_KEY2_REPEAT_TIMER,			(ke_msg_func_t) jwaoo_key_repeat_handler },
	{ JWAOO_KEY2_LONG_CLICK_TIMER,		(ke_msg_func_t) jwaoo_key_long_click_handler },
	{ JWAOO_KEY2_MULTI_CLICK_TIMER,		(ke_msg_func_t) jwaoo_key_multi_click_handler },
	{ JWAOO_KEY3_REPEAT_TIMER,			(ke_msg_func_t) jwaoo_key_repeat_handler },
	{ JWAOO_KEY3_LONG_CLICK_TIMER,		(ke_msg_func_t) jwaoo_key_long_click_handler },
	{ JWAOO_KEY3_MULTI_CLICK_TIMER,		(ke_msg_func_t) jwaoo_key_multi_click_handler },
	{ JWAOO_KEY4_REPEAT_TIMER,			(ke_msg_func_t) jwaoo_key_repeat_handler },
	{ JWAOO_KEY4_LONG_CLICK_TIMER,		(ke_msg_func_t) jwaoo_key_long_click_handler },
	{ JWAOO_KEY4_MULTI_CLICK_TIMER,		(ke_msg_func_t) jwaoo_key_multi_click_handler },
};

static const struct ke_msg_handler jwaoo_app_suspend_handlers[] = {
	{ JWAOO_BATT_POLL,					(ke_msg_func_t) jwaoo_battery_poll_handler },
};

static const struct ke_msg_handler jwaoo_app_deep_sleep_handlers[] = {
	{ KE_MSG_DEFAULT_HANDLER,			(ke_msg_func_t) jwaoo_dummy_handler },
};

static const struct ke_state_handler jwaoo_app_state_handler[JWAOO_APP_STATE_MAX] = {
    [JWAOO_APP_ACTIVE]		= KE_STATE_HANDLER(jwaoo_app_active_handlers),
    [JWAOO_APP_SUSPEND]		= KE_STATE_HANDLER(jwaoo_app_suspend_handlers),
	[JWAOO_APP_DEEP_SLEEP] 	= KE_STATE_HANDLER(jwaoo_app_deep_sleep_handlers),
};

static ke_state_t __attribute__((section("retention_mem_area0"), zero_init)) jwaoo_app_state;

static const struct ke_state_handler jwaoo_app_default_handler = KE_STATE_HANDLER(jwaoo_app_active_handlers);
static const struct ke_task_desc TASK_DESC_JWAOO_APP = { jwaoo_app_state_handler, &jwaoo_app_default_handler, &jwaoo_app_state, JWAOO_APP_STATE_MAX, 1};

void jwaoo_task_init(void)
{
	ke_task_create(TASK_JWAOO_APP, &TASK_DESC_JWAOO_APP);
	ke_state_set(TASK_JWAOO_APP, JWAOO_APP_ACTIVE);
}

void jwaoo_set_deep_sleep_enable(bool enable)
{
	if (jwaoo_deep_sleep == enable) {
		return;
	}

	jwaoo_deep_sleep = enable;

	if (enable) {
		set_pad_deep_sleep(true);
		// ke_state_set(TASK_JWAOO_APP, JWAOO_APP_DEEP_SLEEP);

		arch_ble_ext_wakeup_on();

#if (USE_MEMORY_MAP == EXT_SLEEP_SETUP)
		arch_set_sleep_mode(ARCH_EXT_SLEEP_ON);
#else
		arch_set_sleep_mode(ARCH_DEEP_SLEEP_ON);
#endif
	} else {
		arch_ble_ext_wakeup_off();
		arch_set_sleep_mode(ARCH_SLEEP_OFF);

		if (GetBits16(SYS_STAT_REG, PER_IS_DOWN)) {
			periph_init();
		}

		arch_ble_force_wakeup();
		SetBits16(SYS_CTRL_REG, DEBUGGER_ENABLE, 1);

		if (jwaoo_suspended) {
			// ke_state_set(TASK_JWAOO_APP, JWAOO_APP_SUSPEND);
		}
	}
}

void jwaoo_set_suspend_enable(bool enable, bool force)
{
	if (jwaoo_suspended == enable) {
		return;
	}

	jwaoo_suspended = enable;
	jwaoo_set_deep_sleep_enable(enable);

	if (enable) {
		// ke_state_set(TASK_JWAOO_APP, JWAOO_APP_SUSPEND);
		app_easy_gap_advertise_stop();
	} else {
		// ke_state_set(TASK_JWAOO_APP, JWAOO_APP_ACTIVE);
		user_app_adv_start();
	}
}
