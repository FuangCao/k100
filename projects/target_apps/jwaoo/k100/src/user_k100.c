/**
 ****************************************************************************************
 *
 * @file user_k100.c
 *
 * @brief Barebone project source code.
 *
 * Copyright (C) 2015. Dialog Semiconductor Ltd, unpublished work. This computer
 * program includes Confidential, Proprietary Information and is a Trade Secret of
 * Dialog Semiconductor Ltd.  All use, disclosure, and/or reproduction is prohibited
 * unless authorized in writing. All Rights Reserved.
 *
 * <bluetooth.support@diasemi.com> and contributors.
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"             // SW configuration
#include "user_k100.h"
#include "arch_api.h"
#include "gap.h"
#include "jwaoo_pwm.h"

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

uint8_t app_connection_idx;
ke_msg_id_t app_param_update_request_timer_used;

/**
 ****************************************************************************************
 * @brief Parameter update request timer callback function.
 * @return void
 ****************************************************************************************
*/
static void param_update_request_timer_cb()
{
    app_param_update_request_timer_used = 0xFFFF;

    app_easy_gap_param_update_start(app_connection_idx);
}

void user_app_init(void)
{
	jwaoo_app_init();
    default_app_on_init();
}

void user_app_connection(uint8_t connection_idx, struct gapc_connection_req_ind const *param)
{
    if (app_env[connection_idx].conidx != GAP_INVALID_CONIDX)
    {
        app_connection_idx = connection_idx;

        // Check if the parameters of the established connection are the preferred ones.
        // If not then schedule a connection parameter update request.
        if ((param->con_interval < user_connection_param_conf.intv_min) ||
            (param->con_interval > user_connection_param_conf.intv_max) ||
            (param->con_latency != user_connection_param_conf.latency) ||
            (param->sup_to != user_connection_param_conf.time_out))
        {
            // Connection params are not these that we expect
            app_param_update_request_timer_used = app_easy_timer(APP_PARAM_UPDATE_REQUEST_TO, param_update_request_timer_cb);
        }

		jwaoo_app_set_connect_state(true);
    }
    else
    {
        // No connection has been established, restart advertising
        jwaoo_app_adv_start();
    }

    default_app_on_connection(connection_idx, param);
}

void user_app_adv_undirect_complete(uint8_t status)
{
    // If advertising was canceled then update advertising data and start advertising again
    if (status == GAP_ERR_CANCELED)
    {
        jwaoo_app_adv_start();
    }
}

void user_app_disconnect(struct gapc_disconnect_ind const *param)
{
     uint8_t state = ke_state_get(TASK_APP);

	jwaoo_app_set_connect_state(false);

#if 0
    if ((state == APP_SECURITY) ||
        (state == APP_CONNECTED) ||
        (state == APP_PARAM_UPD))
    {
        // Restart Advertising
        jwaoo_app_adv_start();
    }
    else
    {
        // We are not in a Connected State
        ASSERT_ERR(0);
    }
#else
	jwaoo_app_adv_start();
#endif
}

void user_catch_rest_hndl(ke_msg_id_t const msgid,
                          void const *param,
                          ke_task_id_t const dest_id,
                          ke_task_id_t const src_id)
{
    switch(msgid)
    {
        case GAPC_PARAM_UPDATED_IND:
        {
            // Cast the "param" pointer to the appropriate message structure
            struct gapc_param_updated_ind const *msg_param = (struct gapc_param_updated_ind const *)(param);

            // Check if updated Conn Params filled to preffered ones
            if ((msg_param->con_interval >= user_connection_param_conf.intv_min) &&
                (msg_param->con_interval <= user_connection_param_conf.intv_max) &&
                (msg_param->con_latency == user_connection_param_conf.latency) &&
                (msg_param->sup_to == user_connection_param_conf.time_out))
            {
            }
        } break;
        
        default:
            break;
    }
}
/// @} APP
