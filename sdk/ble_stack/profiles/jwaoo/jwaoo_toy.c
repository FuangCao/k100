/**
 ****************************************************************************************
 *
 * @file jwaoo_toy.c
 *
 * @brief Device Information Service Server Implementation.
 *
 * Copyright (C) RivieraWaves 2009-2013
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup JWAOO_TOY
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"

#if (BLE_JWAOO_TOY_SERVER)
#include "app.h"
#include "pwm.h"
#include "uart.h"
#include "stdarg.h"
#include "spi_flash.h"
#include "attm_util.h"
#include "atts_util.h"
#include "jwaoo_toy.h"
#include "jwaoo_toy_task.h"
#include "prf_utils.h"
#include "app_easy_timer.h"
#include "user_periph_setup.h"

#include "jwaoo_i2c.h"
#include "jwaoo_pwm.h"
#include "jwaoo_app.h"
#include "jwaoo_key.h"
#include "jwaoo_moto.h"
#include "jwaoo_sensor.h"

/*
 * MACROS
 ****************************************************************************************
 */
/*
 * JWAOO_TOY ATTRIBUTES
 ****************************************************************************************
 */

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

struct jwaoo_toy_env_tag jwaoo_toy_env __attribute__((section("retention_mem_area0"), zero_init)); //@RETENTION MEMORY

static const struct ke_task_desc TASK_DESC_JWAOO_TOY = {
	.state_handler = jwaoo_toy_state_handler,
	.default_handler = &jwaoo_toy_default_handler,
	.state = jwaoo_toy_state,
	.state_max = JWAOO_TOY_STATE_COUNT,
	.idx_max = JWAOO_TOY_TASK_COUNT
};

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

void jwaoo_toy_init(void)
{
    // Reset environment
    memset(&jwaoo_toy_env, 0, sizeof(jwaoo_toy_env));

    // Create JWAOO_TOY task
    ke_task_create(TASK_JWAOO_TOY, &TASK_DESC_JWAOO_TOY);

    // Set task in disabled state
    ke_state_set(TASK_JWAOO_TOY, JWAOO_TOY_DISABLED);
}

void jwaoo_toy_enable(uint16_t conhdl) 
{
	jwaoo_app_env.sensor_capacity_dead = 0;
	jwaoo_app_env.sensor_accel_dead = 0;
}

void jwaoo_toy_disable(uint16_t conhdl) 
{
    // Inform the application about the disconnection
    struct jwaoo_toy_disable_ind *ind = KE_MSG_ALLOC(JWAOO_TOY_DISABLE_IND,
		jwaoo_toy_env.con_info.appid, TASK_JWAOO_TOY, jwaoo_toy_disable_ind);

    ind->conhdl = conhdl;

    ke_msg_send(ind);

	jwaoo_toy_env.notify_busy_mask = 0;
	jwaoo_toy_env.notify_enable_mask = 0;

	jwaoo_app_env.flash_write_enable = false;
	jwaoo_app_env.flash_write_success = false;

	jwaoo_app_env.battery_report = false;

	jwaoo_sensor_set_enable(false);
	jwaoo_app_set_upgrade_enable(false);
	jwaoo_app_set_factory_enable(false);

    //Disable JWAOO_TOY in database
    attmdb_svc_set_permission(jwaoo_toy_env.handle, PERM(SVC, DISABLE));

    //Go to idle state
    ke_state_set(TASK_JWAOO_TOY, JWAOO_TOY_IDLE);
}

// ===============================================================================

uint8_t jwaoo_toy_write_data(uint16_t attr, const void *data, int size)
{
	return attmdb_att_set_value(jwaoo_toy_env.handle + attr, size, (void *) data);
}

uint8_t jwaoo_toy_send_notify(uint16_t attr, const void *data, int size)
{
	uint8_t ret;
	uint16_t handle;
	uint16_t mask = 1 << attr;

	if ((jwaoo_toy_env.notify_enable_mask & mask) == 0) {
		return ATT_ERR_WRITE_NOT_PERMITTED;
	}

	if (jwaoo_toy_env.notify_busy_mask & mask) {
		return ATT_ERR_PREPARE_QUEUE_FULL;
	}

	jwaoo_toy_env.notify_busy_mask |= mask;

	handle = jwaoo_toy_env.handle + attr;

	ret = attmdb_att_set_value(handle, size, (uint8_t *) data);
	if (ret != ATT_ERR_NO_ERROR) {
		println("Failed to attmdb_att_set_value: %d", ret);
		return ret;
	}

	prf_server_send_event((prf_env_struct *) &jwaoo_toy_env, false, handle);

	return ATT_ERR_NO_ERROR;
}

uint8_t jwaoo_toy_send_command_u8(uint8_t type, uint8_t value)
{
	struct jwaoo_toy_command command = {
		.type = type,
		.value8 = value
	};

	return jwaoo_toy_send_command((uint8_t *) &command, 2);
}

uint8_t jwaoo_toy_send_command_u16(uint8_t type, uint16_t value)
{
	struct jwaoo_toy_command command = {
		.type = type,
		.value16 = value
	};

	return jwaoo_toy_send_command((uint8_t *) &command, 3);
}

uint8_t jwaoo_toy_send_command_u32(uint8_t type, uint32_t value)
{
	struct jwaoo_toy_command command = {
		.type = type,
		.value32 = value
	};

	return jwaoo_toy_send_command((uint8_t *) &command, 5);
}

uint8_t jwaoo_toy_send_command_data(uint8_t type, const uint8_t *data, int size)
{
	uint8_t buff[size + 1];
	struct jwaoo_toy_command *command = (struct jwaoo_toy_command *) buff;

	command->type = type;
	memcpy(command->bytes, data, size);

	return jwaoo_toy_send_command(buff, sizeof(buff));
}

uint8_t jwaoo_toy_send_command_text(uint8_t type, const char *fmt, ...)
{
	va_list ap;
	int length;
	uint8_t buff[JWAOO_TOY_MAX_COMMAND_SIZE];

	buff[0] = type;

	va_start(ap, fmt);
	length = vsnprintf((char *) buff + 1, sizeof(buff) - 1, fmt, ap);
	va_end(ap);

	return jwaoo_toy_send_command(buff, length + 1);
}

// ===============================================================================

uint8_t jwaoo_toy_send_response_u8_typed(uint8_t type, uint8_t command, uint8_t value)
{
	struct jwaoo_toy_response response = {
		.command = command,
		.type = type,
		.value8 = value
	};

	return jwaoo_toy_send_command(&response, 3);
}

uint8_t jwaoo_toy_send_response_u8(uint8_t command, uint8_t value)
{
	return jwaoo_toy_send_response_u8_typed(JWAOO_TOY_RSP_U8, command, value);
}

uint8_t jwaoo_toy_send_response_bool(uint8_t command, bool value)
{
	return jwaoo_toy_send_response_u8_typed(JWAOO_TOY_RSP_BOOL, command, value);
}

uint8_t jwaoo_toy_send_response_u16(uint8_t command, uint16_t value)
{
	struct jwaoo_toy_response response = {
		.command = command,
		.type = JWAOO_TOY_RSP_U16,
		.value16 = value
	};

	return jwaoo_toy_send_command(&response, 4);
}

uint8_t jwaoo_toy_send_response_u32(uint8_t command, uint32_t value)
{
	struct jwaoo_toy_response response = {
		.command = command,
		.type = JWAOO_TOY_RSP_U32,
		.value32 = value
	};

	return jwaoo_toy_send_command(&response, 6);
}

uint8_t jwaoo_toy_send_response_data(uint8_t command, const uint8_t *data, uint16_t size)
{
	char buff[size + 2];
	struct jwaoo_toy_response *response = (void *) buff;

	response->command = command;
	response->type = JWAOO_TOY_RSP_DATA;
	memcpy(response->bytes, data, size);

	return jwaoo_toy_send_command(buff, sizeof(buff));
}

uint8_t jwaoo_toy_send_response_text(uint8_t command, const char *fmt, ...)
{
	va_list ap;
	int length;
	char buff[JWAOO_TOY_MAX_COMMAND_SIZE];
	struct jwaoo_toy_response *response = (void *) buff;

	response->command = command;
	response->type = JWAOO_TOY_RSP_TEXT;

	va_start(ap, fmt);
	length = vsnprintf(response->text, sizeof(buff) - 2, fmt, ap);
	va_end(ap);

	return jwaoo_toy_send_command(buff, length + 2);
}

// ===============================================================================

void jwaoo_toy_process_command(const struct jwaoo_toy_command *command, uint16_t length)
{
	bool success = false;

	if (length < 1) {
		println("Invalid command length: %d", length);
		return;
	}

	println("command = %d, length = %d", command->type, length);

	switch (command->type) {
	case JWAOO_TOY_CMD_NOOP:
		success = true;
		break;

	case JWAOO_TOY_CMD_IDENTIFY:
		jwaoo_toy_send_response_text(command->type, "%s", JWAOO_TOY_IDENTIFY);
		return;

	case JWAOO_TOY_CMD_VERSION:
		jwaoo_toy_send_response_u32(command->type, JWAOO_TOY_VERSION);
		return;

	case JWAOO_TOY_CMD_BUILD_DATE:
		jwaoo_toy_send_response_text(command->type, "%s %s", __DATE__, __TIME__);
		return;

	case JWAOO_TOY_CMD_REBOOT:
		jwaoo_app_timer_set(JWAOO_REBOOT, 100);
		success = true;
		break;

	case JWAOO_TOY_CMD_SHUTDOWN:
		jwaoo_app_timer_set(JWAOO_SHUTDOWN, 100);
		success = true;
		break;

	case JWAOO_TOY_CMD_I2C_RW:
		if (length >= 3) {
			int count = 0;
			uint8_t rdlen = command->i2c.rdlen;
			uint8_t wrlen = length - 3;
			struct jwaoo_i2c_message msgs[2];
			uint8_t response[rdlen + 2];

			if (wrlen > 0) {
				msgs[count].read = 0;
				msgs[count].count = wrlen;
				msgs[count].data = (uint8_t *) command->i2c.data;
				count++;
			}

			if (rdlen > 0) {
				msgs[count].read = 1;
				msgs[count].count = rdlen;
				msgs[count].data = response + 2;
				count++;
			}

			println("i2c: rdlen = %d, wrlen = %d, count = %d", rdlen, wrlen, count);

			if (count < 1 || jwaoo_i2c_transfer(command->i2c.slave, msgs, count) < 0) {
				break;
			}

			response[0] = command->type;
			response[1] = JWAOO_TOY_RSP_DATA;
			jwaoo_toy_send_command(response, sizeof(response));
			return;
		}
		break;

	case JWAOO_TOY_CMD_SUSPEND_DELAY:
		if (length == 1) {
			jwaoo_toy_send_response_u16(command->type, jwaoo_app_settings.suspend_delay);
			return;
		}

		if (length != 3 || command->value16 < 10) {
			break;
		}

		jwaoo_app_settings.suspend_delay = command->value16;
		success = true;
		break;

	case JWAOO_TOY_CMD_APP_DATA:
		if (length > 1) {
			if (--length > sizeof(jwaoo_app_env.app_data)) {
				break;
			}

			memcpy(jwaoo_app_env.app_data, command->bytes, length);
			success = true;
			break;
		}

		jwaoo_toy_send_response_data(command->type, jwaoo_app_env.app_data, sizeof(jwaoo_app_env.app_data));
		return;

	case JWAOO_TOY_CMD_APP_SETTINGS:
		if (length == 1) {
			jwaoo_toy_send_response_data(command->type, (uint8_t *) &jwaoo_app_settings, sizeof(jwaoo_app_settings));
			return;
		}

		if (--length == sizeof(jwaoo_app_settings)) {
			memcpy(&jwaoo_app_settings, command->bytes, length);
			success = true;
		}
		break;

	case JWAOO_TOY_CMD_FACTORY_ENABLE:
		jwaoo_app_set_factory_enable(length > 1 && command->enable.value);
		success = true;
		break;

	case JWAOO_TOY_CMD_LED_ENABLE:
		if (length >= 2) {
			bool enable = (length > 2 && command->led.enable > 0);

			if (command->led.index == 1) {
				if (enable) {
					jwaoo_pwm_blink_open(JWAOO_PWM_BATT_LED);
				} else {
					jwaoo_pwm_blink_close(JWAOO_PWM_BATT_LED);
				}
			} else if (command->led.index == 2) {
				if (enable) {
					BT_LED_OPEN;
				} else {
					BT_LED_CLOSE;
				}
			} else {
				break;
			}

			success = true;
		}
		break;

	// ================================================================================

	case JWAOO_TOY_CMD_BATT_EVENT_ENABLE:
		jwaoo_app_env.battery_report = (length > 1 && command->enable.value);
		success = true;
		break;

	case JWAOO_TOY_CMD_BATT_INFO: {
		struct jwaoo_toy_response response = {
			.command = command->type,
			.type = JWAOO_TOY_RSP_DATA,
			.battery.state = jwaoo_app_env.battery_state,
			.battery.level = jwaoo_app_env.battery_level,
			.battery.voltage = jwaoo_app_env.battery_voltage,
		};

		jwaoo_toy_send_command(&response, 6);
		return;
	}

	case JWAOO_TOY_CMD_BATT_SHUTDOWN_VOLTAGE:
		if (length > 1) {
			if (length != 3 || command->value16 > JWAOO_BATT_VOLTAGE_SHUTDOWN_MAX) {
				break;
			}

			jwaoo_app_settings.shutdown_voltage = command->value16;
			success = true;
			break;
		}

		jwaoo_toy_send_response_u16(command->type, jwaoo_app_settings.shutdown_voltage);
		return;

	// ================================================================================

	case JWAOO_TOY_CMD_SENSOR_ENABLE:
		if (jwaoo_app_is_upgrade()) {
			break;
		}

		if (length > 1 && command->enable.value) {
			if (length > 5) {
				jwaoo_app_env.sensor_poll_delay = command->enable.delay32 / 10;
			}

			success = jwaoo_sensor_set_enable(true);
			jwaoo_app_env.sensor_enable = success;
		} else {
			success = jwaoo_sensor_set_enable(false);
			jwaoo_app_env.sensor_enable = false;
		}
		break;

	// ================================================================================

	case JWAOO_TOY_CMD_MOTO_SET_MODE:
		if (jwaoo_app_is_upgrade() || length != 3) {
			break;
		}

		if (command->moto.mode < JWAOO_MOTO_MODE_COUNT) {
			if (command->moto.level > 0) {
				jwaoo_moto_set_mode(command->moto.mode, command->moto.level);
			} else {
				jwaoo_moto_blink_close();
			}

			success = true;
		}
		break;

	case JWAOO_TOY_CMD_MOTO_GET_MODE: {
		struct jwaoo_toy_response response = {
			.command = command->type,
			.type = JWAOO_TOY_RSP_DATA,
			.moto.mode = jwaoo_app_env.moto_mode,
			.moto.level = jwaoo_moto_get_speed(),
		};

		jwaoo_toy_send_command(&response, 4);
		return;
	}

	case JWAOO_TOY_CMD_MOTO_EVENT_ENABLE:
		jwaoo_app_env.moto_report = (length > 1 && command->enable.value);
		success = true;
		break;

	// ================================================================================

	case JWAOO_TOY_CMD_KEY_CLICK_ENABLE:
		jwaoo_app_env.key_click_enable = (length > 1 && command->enable.value);
		success = true;
		break;

	case JWAOO_TOY_CMD_KEY_LONG_CLICK_ENABLE:
		if (length > 1 && command->enable.value) {
			jwaoo_app_env.key_long_click_enable = true;

			if (length > 3) {
				uint16_t delay = command->enable.delay16 / 10;

				if (delay > 0) {
					jwaoo_app_env.key_long_click_delay = delay;
				}
			}
		} else {
			jwaoo_app_env.key_long_click_enable = false;
		}

		success = true;
		break;

	case JWAOO_TOY_CMD_KEY_MULTI_CLICK_ENABLE:
		if (length > 1 && command->enable.value) {
			jwaoo_app_env.key_multi_click_enable = true;

			if (length > 3) {
				uint16_t delay = command->enable.delay16 / 10;

				if (delay > 0) {
					jwaoo_app_env.key_multi_click_delay = delay;
				}
			}
		} else {
			jwaoo_app_env.key_multi_click_enable = false;
		}

		success = true;
		break;

	case JWAOO_TOY_CMD_KEY_LOCK:
		if (length > 1) {
			jwaoo_app_env.key_app_locked = command->enable.value;
			success = true;
			break;
		}

		jwaoo_toy_send_response_u8(command->type, jwaoo_app_env.key_app_locked);
		return;

	// ================================================================================

	case JWAOO_TOY_CMD_GPIO_GET:
		if (length == 3) {
			uint8_t value = GPIO_GetPinStatus((GPIO_PORT) command->gpio.port, (GPIO_PIN) command->gpio.pin);
			jwaoo_toy_send_response_u8(command->type, value);
			return;
		}
		break;

	case JWAOO_TOY_CMD_GPIO_SET:
		if (length == 4) {
			if (command->gpio.value) {
				GPIO_SetActive((GPIO_PORT) command->gpio.port, (GPIO_PIN) command->gpio.pin);
			} else {
				GPIO_SetInactive((GPIO_PORT) command->gpio.port, (GPIO_PIN) command->gpio.pin);
			}

			success = true;
		}
		break;

	case JWAOO_TOY_CMD_GPIO_CFG:
		if (length == 6) {
			GPIO_ConfigurePin((GPIO_PORT) command->gpio_config.port, (GPIO_PIN) command->gpio_config.pin,
				(GPIO_PUPD) command->gpio_config.mode, (GPIO_FUNCTION) command->gpio_config.function, command->gpio_config.high > 0);
			success = true;
		}
		break;

	// ================================================================================

	default:
		println("Invalid command: %d", command->type);
		break;
	}

	jwaoo_toy_send_response_bool(command->type, success);
}

static bool jwaoo_toy_process_flash_data_safe(const uint8_t *data, int length)
{
	return true;
}

bool jwaoo_toy_process_flash_data(const uint8_t *data, uint16_t length)
{
	if (jwaoo_app_env.flash_write_enable) {
		if (jwaoo_toy_process_flash_data_safe(data, length)) {
			return true;
		}

		println("Failed to jwaoo_toy_process_flash_data_safe");
		jwaoo_app_env.flash_write_enable = false;
	} else {
		println("write flash is not enable");
	}

	jwaoo_app_env.flash_write_success = false;

	return false;
}

// ================================================================================

void jwaoo_toy_report_key_state(struct jwaoo_key_device *key)
{
	struct jwaoo_toy_key_message *msg = KE_MSG_ALLOC(JWAOO_TOY_KEY_REPORT_STATE, TASK_JWAOO_TOY, TASK_APP, jwaoo_toy_key_message);

	msg->key = key;
	msg->value = key->value;

	ke_msg_send(msg);
}

void jwaoo_toy_report_key_click(struct jwaoo_key_device *key, uint8_t count)
{
	struct jwaoo_toy_key_message *msg = KE_MSG_ALLOC(JWAOO_TOY_KEY_REPORT_CLICK, TASK_JWAOO_TOY, TASK_APP, jwaoo_toy_key_message);

	msg->key = key;
	msg->count = count;

	ke_msg_send(msg);
}

void jwaoo_toy_report_key_long_click(struct jwaoo_key_device *key)
{
	struct jwaoo_toy_key_message *msg = KE_MSG_ALLOC(JWAOO_TOY_KEY_REPORT_LONG_CLICK, TASK_JWAOO_TOY, TASK_APP, jwaoo_toy_key_message);

	msg->key = key;

	ke_msg_send(msg);
}

#endif //BLE_JWAOO_TOY_SERVER

/// @} JWAOO_TOY
