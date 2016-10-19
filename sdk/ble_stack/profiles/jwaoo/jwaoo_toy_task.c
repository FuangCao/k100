/**
 ****************************************************************************************
 *
 * @file jwaoo_toy_task.c
 *
 * @brief Device Information Service Task implementation.
 *
 * Copyright (C) RivieraWaves 2009-2013
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup JWAOO_TOY_TASK
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"

#if (BLE_JWAOO_TOY_SERVER)
#include "gap.h"
#include "pwm.h"
#include "uart.h"
#include "gattc_task.h"
#include "jwaoo_toy.h"
#include "jwaoo_toy_task.h"
#include "prf_utils.h"
#include "attm_util.h"
#include "atts_util.h"
#include "user_periph_setup.h"

#include "jwaoo_app.h"
#include "jwaoo_key.h"
#include "jwaoo_moto.h"
#include "jwaoo_sensor.h"

static uint16_t jwaoo_toy_svc = JWAOO_TOY_UUID_SVC;
static struct att_char_desc jwaoo_toy_command_char = ATT_CHAR(ATT_CHAR_PROP_WR | ATT_CHAR_PROP_RD, 0, JWAOO_TOY_UUID_COMMAND);
static struct att_char_desc jwaoo_toy_event_char = ATT_CHAR(ATT_CHAR_PROP_NTF, 0, JWAOO_TOY_UUID_EVENT);
static struct att_char_desc jwaoo_toy_flash_char = ATT_CHAR(ATT_CHAR_PROP_WR | ATT_CHAR_PROP_RD, 0, JWAOO_TOY_UUID_FLASH);
static struct att_char_desc jwaoo_toy_sensor_char = ATT_CHAR(ATT_CHAR_PROP_NTF, 0, JWAOO_TOY_UUID_SENSOR);
static struct att_char_desc jwaoo_toy_debug_char = ATT_CHAR(ATT_CHAR_PROP_WR | ATT_CHAR_PROP_NTF, 0, JWAOO_TOY_UUID_DEBUG);

const struct attm_desc jwaoo_toy_att_db[JWAOO_TOY_ATTR_COUNT] =
{
	[JWAOO_TOY_ATTR_SVC] = {
		.uuid = ATT_DECL_PRIMARY_SERVICE,
		.perm = PERM(RD, ENABLE),
		.max_length = sizeof(jwaoo_toy_svc),
		.length = sizeof(jwaoo_toy_svc),
		.value = (uint8_t *) &jwaoo_toy_svc
	},
	// ============================================================
	[JWAOO_TOY_ATTR_COMMAND_CHAR] = {
		.uuid = ATT_DECL_CHARACTERISTIC,
		.perm = PERM(RD, ENABLE),
		.max_length = sizeof(jwaoo_toy_command_char),
		.length = sizeof(jwaoo_toy_command_char),
		.value = (uint8_t *) &jwaoo_toy_command_char
	},
	[JWAOO_TOY_ATTR_COMMAND_DATA] = {
		.uuid = JWAOO_TOY_UUID_COMMAND,
		.perm = PERM(WR, ENABLE) | PERM(RD, ENABLE),
		.max_length = JWAOO_TOY_MAX_COMMAND_SIZE,
		.length = 0,
		.value = NULL
	},
	// ============================================================
	[JWAOO_TOY_ATTR_EVENT_CHAR] = {
		.uuid = ATT_DECL_CHARACTERISTIC,
		.perm = PERM(RD, ENABLE),
		.max_length = sizeof(jwaoo_toy_event_char),
		.length = sizeof(jwaoo_toy_event_char),
		.value = (uint8_t *) &jwaoo_toy_event_char
	},
	[JWAOO_TOY_ATTR_EVENT_DATA] = {
		.uuid = JWAOO_TOY_UUID_EVENT,
		.perm = PERM(NTF, ENABLE),
		.max_length = JWAOO_TOY_MAX_EVENT_SIZE,
		.length = 0,
		.value = NULL
	},
	[JWAOO_TOY_ATTR_EVENT_CFG] = {
		.uuid = ATT_DESC_CLIENT_CHAR_CFG,
		.perm = PERM(WR, ENABLE),
		.max_length = 2,
		.length = 0,
		.value = NULL
	},
	// ============================================================
	[JWAOO_TOY_ATTR_FLASH_CHAR] = {
		.uuid = ATT_DECL_CHARACTERISTIC,
		.perm = PERM(RD, ENABLE),
		.max_length = sizeof(jwaoo_toy_flash_char),
		.length = sizeof(jwaoo_toy_flash_char),
		.value = (uint8_t *)& jwaoo_toy_flash_char
	},
	[JWAOO_TOY_ATTR_FLASH_DATA] = {
		.uuid = JWAOO_TOY_UUID_FLASH,
		.perm = PERM(WR, ENABLE) | PERM(RD, ENABLE),
		.max_length = JWAOO_TOY_MAX_FLASH_DATA_SIZE,
		.length = 0,
		.value = NULL
	},
	// ============================================================
	[JWAOO_TOY_ATTR_SENSOR_CHAR] = {
		.uuid = ATT_DECL_CHARACTERISTIC,
		.perm = PERM(RD, ENABLE),
		.max_length = sizeof(jwaoo_toy_sensor_char),
		.length = sizeof(jwaoo_toy_sensor_char),
		.value = (uint8_t *)& jwaoo_toy_sensor_char
	},
	[JWAOO_TOY_ATTR_SENSOR_DATA] = {
		.uuid = JWAOO_TOY_UUID_SENSOR,
		.perm = PERM(NTF, ENABLE),
		.max_length = JWAOO_TOY_MAX_SENSOR_DATA_SIZE,
		.length = 0,
		.value = NULL
	},
	[JWAOO_TOY_ATTR_SENSOR_CFG] = {
		.uuid = ATT_DESC_CLIENT_CHAR_CFG,
		.perm = PERM(WR, ENABLE),
		.max_length = 2,
		.length = 0,
		.value = NULL
	},
	// ============================================================
	[JWAOO_TOY_ATTR_DEBUG_CHAR] = {
		.uuid = ATT_DECL_CHARACTERISTIC,
		.perm = PERM(RD, ENABLE),
		.max_length = sizeof(jwaoo_toy_debug_char),
		.length = sizeof(jwaoo_toy_debug_char),
		.value = (uint8_t *)& jwaoo_toy_debug_char
	},
	[JWAOO_TOY_ATTR_DEBUG_DATA] = {
		.uuid = JWAOO_TOY_UUID_DEBUG,
		.perm = PERM(WR, ENABLE) | PERM(NTF, ENABLE),
		.max_length = JWAOO_TOY_MAX_DEBUG_DATA_SIZE,
		.length = 0,
		.value = NULL
	},
	[JWAOO_TOY_ATTR_DEBUG_CFG] = {
		.uuid = ATT_DESC_CLIENT_CHAR_CFG,
		.perm = PERM(WR, ENABLE),
		.max_length = 2,
		.length = 0,
		.value = NULL
	},
};

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

static int jwaoo_toy_create_db_req_handler(ke_msg_id_t const msgid,
                                      struct jwaoo_toy_create_db_req const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    struct jwaoo_toy_create_db_cfm *cfm;
    //DB Creation Statis
    uint8_t status = ATT_ERR_NO_ERROR;
	//Service content flag
    uint32_t cfg_flag = (1 << JWAOO_TOY_ATTR_COUNT) - 1;

    //Save profile id
    jwaoo_toy_env.con_info.prf_id = TASK_JWAOO_TOY;

	status = attm_svc_create_db(&jwaoo_toy_env.handle, (uint8_t *) &cfg_flag, JWAOO_TOY_ATTR_COUNT, NULL, dest_id, jwaoo_toy_att_db);
	if (status == ATT_ERR_NO_ERROR)
    {
        //Disable service
        status = attmdb_svc_set_permission(jwaoo_toy_env.handle, PERM(SVC, DISABLE));

        //If we are here, database has been fulfilled with success, go to idle test
        ke_state_set(TASK_JWAOO_TOY, JWAOO_TOY_IDLE);
    }

    //Send response to application
    cfm = KE_MSG_ALLOC(JWAOO_TOY_CREATE_DB_CFM, src_id, TASK_JWAOO_TOY, jwaoo_toy_create_db_cfm);
    cfm->status = status;
    ke_msg_send(cfm);

    return (KE_MSG_CONSUMED);
}

static int jwaoo_toy_upgrade_complete_handler(ke_msg_id_t const msgid,
                                         void *param,
                                         ke_task_id_t const dest_id,
                                         ke_task_id_t const src_id)
{
    return (KE_MSG_CONSUMED);
}

static int jwaoo_toy_sensor_poll_handler(ke_msg_id_t const msgid,
                                         void *param,
                                         ke_task_id_t const dest_id,
                                         ke_task_id_t const src_id)
{
	if (jwaoo_app_env.sensor_poll_enable) {
		ke_timer_set(JWAOO_TOY_SENSOR_POLL, TASK_JWAOO_TOY, jwaoo_app_env.sensor_poll_delay);

		if (jwaoo_toy_sensor_notify_busy()) {
			jwaoo_app_env.sensor_pending = true;
		} else {
			jwaoo_sensor_poll();
		}
	}

    return (KE_MSG_CONSUMED);
}

static int jwaoo_toy_battery_report_state_handler(ke_msg_id_t const msgid,
                                         struct jwaoo_toy_key_message const *param,
                                         ke_task_id_t const dest_id,
                                         ke_task_id_t const src_id)
{
	struct jwaoo_toy_command command = {
		.type = JWAOO_TOY_EVT_BATT_INFO,
		.battery.state = jwaoo_app_env.battery_state,
		.battery.level = jwaoo_app_env.battery_level,
		.battery.voltage = jwaoo_app_env.battery_voltage,
	};

	jwaoo_toy_send_event(&command, 5);

	return (KE_MSG_CONSUMED);
}

static int jwaoo_toy_key_report_state_handler(ke_msg_id_t const msgid,
                                         struct jwaoo_toy_key_message const *param,
                                         ke_task_id_t const dest_id,
                                         ke_task_id_t const src_id)
{
	uint8_t event[] = { JWAOO_TOY_EVT_KEY_STATE, param->key->code, param->value };

	jwaoo_toy_send_event(event, sizeof(event));

	return (KE_MSG_CONSUMED);
}

static int jwaoo_toy_key_report_click_handler(ke_msg_id_t const msgid,
                                         struct jwaoo_toy_key_message const *param,
                                         ke_task_id_t const dest_id,
                                         ke_task_id_t const src_id)
{
	uint8_t event[] = { JWAOO_TOY_EVT_KEY_CLICK, param->key->code, param->count };

	jwaoo_toy_send_event(event, sizeof(event));

	return (KE_MSG_CONSUMED);
}

static int jwaoo_toy_key_report_long_click_handler(ke_msg_id_t const msgid,
                                         struct jwaoo_toy_key_message const *param,
                                         ke_task_id_t const dest_id,
                                         ke_task_id_t const src_id)
{
	uint8_t event[] = { JWAOO_TOY_EVT_KEY_LONG_CLICK, param->key->code };

	jwaoo_toy_send_event(event, sizeof(event));

	return (KE_MSG_CONSUMED);
}

static int jwaoo_toy_moto_report_state_handler(ke_msg_id_t const msgid,
										 struct jwaoo_toy_key_message const *param,
										 ke_task_id_t const dest_id,
										 ke_task_id_t const src_id)
{
	struct jwaoo_toy_command command = {
		.type = JWAOO_TOY_EVT_MOTO_STATE_CHANGED,
		.moto.mode = jwaoo_app_env.moto_mode,
		.moto.level = jwaoo_moto_get_speed(),
	};

	jwaoo_toy_send_event(&command, 3);

	return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref JWAOO_TOY_ENABLE_REQ message.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int jwaoo_toy_enable_req_handler(ke_msg_id_t const msgid,
                                   struct jwaoo_toy_enable_req const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    //Save the connection handle associated to the profile
    jwaoo_toy_env.con_info.conidx = gapc_get_conidx(param->conhdl);
    //Save the application id
    jwaoo_toy_env.con_info.appid = src_id;

    // Check if the provided connection exist
    if (jwaoo_toy_env.con_info.conidx == GAP_INVALID_CONIDX)
    {
        // The connection doesn't exist, request disallowed
        prf_server_error_ind_send((prf_env_struct *) &jwaoo_toy_env, PRF_ERR_REQ_DISALLOWED,
                                  JWAOO_TOY_ERROR_IND, JWAOO_TOY_ENABLE_REQ);
    }
    else
    {
        //Enable Attributes + Set Security Level
        attmdb_svc_set_permission(jwaoo_toy_env.handle, param->sec_lvl);

        // Go to connected state
        ke_state_set(TASK_JWAOO_TOY, JWAOO_TOY_CONNECTED);
        jwaoo_toy_enable(param->conhdl);
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Disconnection indication to HTPT.
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance
 * @param[in] src_id    ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gapc_disconnect_ind_handler(ke_msg_id_t const msgid,
                                      struct gapc_disconnect_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    if (KE_IDX_GET(src_id) == jwaoo_toy_env.con_info.conidx)
    {
        jwaoo_toy_disable(param->conhdl);
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref gattc_cmp_evt message.
 * The handler enables the Serial Port Service Device profile.
 * @param[in] msgid Id of the message received .
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance 
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gattc_cmp_evt_handler(ke_msg_id_t const msgid,
                                struct gattc_cmp_evt const *param,
                                ke_task_id_t const dest_id,
                                ke_task_id_t const src_id)
{
	if (param->req_type == GATTC_NOTIFY) {
		jwaoo_toy_env.notify_busy_mask = 0;

		if (jwaoo_app_env.sensor_pending) {
			jwaoo_app_env.sensor_pending = false;
			jwaoo_sensor_poll();
		}
	}

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref GATT_WRITE_CMD_IND message.
 * Receive and proces incoming data 
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gattc_write_cmd_ind_handler(ke_msg_id_t const msgid,
                                      struct gattc_write_cmd_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
	int attr = param->handle - jwaoo_toy_env.handle;

	switch (attr) {
	case JWAOO_TOY_ATTR_COMMAND_DATA:
		jwaoo_toy_process_command((struct jwaoo_toy_command *) param->value, param->length);
		break;

	case JWAOO_TOY_ATTR_FLASH_DATA:
		jwaoo_toy_process_flash_data(param->value, param->length);
		break;

	case JWAOO_TOY_ATTR_EVENT_CFG:
	case JWAOO_TOY_ATTR_SENSOR_CFG:
	case JWAOO_TOY_ATTR_DEBUG_CFG:
		if (param->length == 2) {
			uint16_t mask = 1 << (attr - 1);

			if (param->value[0]) {
				jwaoo_toy_env.notify_enable_mask |= mask;
			} else {
				jwaoo_toy_env.notify_enable_mask &= ~(mask);
			}

			println("notify_enable_mask = %04x", jwaoo_toy_env.notify_enable_mask);
		}
		break;
	}

	if (param->response) {
		atts_write_rsp_send(jwaoo_toy_env.con_info.conidx, param->handle, PRF_ERR_OK);
	}

    return (KE_MSG_CONSUMED);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

///Disabled State handler definition.
const struct ke_msg_handler jwaoo_toy_disabled[] =
{
	{ JWAOO_TOY_CREATE_DB_REQ,		(ke_msg_func_t) jwaoo_toy_create_db_req_handler },
};

///Idle State handler definition.
const struct ke_msg_handler jwaoo_toy_idle[] =
{
    { JWAOO_TOY_ENABLE_REQ,			(ke_msg_func_t) jwaoo_toy_enable_req_handler },
};

const struct ke_msg_handler jwaoo_toy_connected[] =
{
	{ JWAOO_TOY_SENSOR_POLL,			(ke_msg_func_t) jwaoo_toy_sensor_poll_handler },
	{ JWAOO_TOY_UPGRADE_COMPLETE,		(ke_msg_func_t) jwaoo_toy_upgrade_complete_handler },
	{ JWAOO_TOY_BATT_REPORT_STATE,		(ke_msg_func_t) jwaoo_toy_battery_report_state_handler },
	{ JWAOO_TOY_KEY_REPORT_STATE,		(ke_msg_func_t) jwaoo_toy_key_report_state_handler },
	{ JWAOO_TOY_KEY_REPORT_CLICK,		(ke_msg_func_t) jwaoo_toy_key_report_click_handler },
	{ JWAOO_TOY_KEY_REPORT_LONG_CLICK,	(ke_msg_func_t) jwaoo_toy_key_report_long_click_handler },
	{ JWAOO_TOY_MOTO_REPORT_STATE,		(ke_msg_func_t) jwaoo_toy_moto_report_state_handler },
	{ GAPC_DISCONNECT_IND,				(ke_msg_func_t) gapc_disconnect_ind_handler },
    { GATTC_CMP_EVT,					(ke_msg_func_t) gattc_cmp_evt_handler },
    { GATTC_WRITE_CMD_IND,				(ke_msg_func_t) gattc_write_cmd_ind_handler },
};

///Specifies the message handler structure for every input state.
const struct ke_state_handler jwaoo_toy_state_handler[JWAOO_TOY_STATE_COUNT] =
{
    [JWAOO_TOY_DISABLED]			= KE_STATE_HANDLER(jwaoo_toy_disabled),
    [JWAOO_TOY_IDLE]				= KE_STATE_HANDLER(jwaoo_toy_idle),
    [JWAOO_TOY_CONNECTED]			= KE_STATE_HANDLER(jwaoo_toy_connected),
};

///Specifies the message handlers that are common to all states.
const struct ke_state_handler jwaoo_toy_default_handler = KE_STATE_HANDLER_NONE;

///Defines the place holder for the states of all the task instances.
ke_state_t jwaoo_toy_state[JWAOO_TOY_TASK_COUNT] __attribute__((section("retention_mem_area0"), zero_init)); //@RETENTION MEMORY

#endif //BLE_JWAOO_TOY_SERVER

/// @} JWAOO_TOY_TASK
