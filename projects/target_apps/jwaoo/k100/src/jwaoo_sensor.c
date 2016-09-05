#include "jwaoo_toy_task.h"
#include "jwaoo_sensor.h"
#include "jwaoo_app.h"
#include "jwaoo_toy.h"
#include "fdc1004.h"
#include "bmi160.h"

static bool jwaoo_sensor_read_values_dummy(uint8_t values[3])
{
	return false;
}

static bool (*jwaoo_accel_sensor_set_enable)(bool enable);
static bool (*jwaoo_capacity_sensor_set_enable)(bool enable);
static bool (*jwaoo_accel_sensor_read_values)(uint8_t values[3]) = jwaoo_sensor_read_values_dummy;
static bool (*jwaoo_capacity_sensor_read_values)(uint8_t values[4]) = jwaoo_sensor_read_values_dummy;

static bool jwaoo_sensor_set_enable_retry(bool (*handler)(bool enable))
{
	int i;

	for (i = 10; i > 0; i--) {
		if (handler(true)) {
			return true;
		}
	}

	return false;
}

bool jwaoo_sensor_set_enable(bool enable)
{
	println("sensor_enable = %d, sensor_poll_delay = %d", enable, jwaoo_app_env.sensor_poll_delay);

	if (enable) {
		LDO_P3V3_OPEN;

		if (jwaoo_accel_sensor_set_enable) {
			jwaoo_sensor_set_enable_retry(jwaoo_accel_sensor_set_enable);
		} else if (jwaoo_sensor_set_enable_retry(bmi160_set_enable)) {
			jwaoo_accel_sensor_set_enable = bmi160_set_enable;
			jwaoo_accel_sensor_read_values = bmi160_read_sensor_values;
		} else {
			jwaoo_accel_sensor_read_values = jwaoo_sensor_read_values_dummy;
		}

		if (jwaoo_capacity_sensor_set_enable) {
			jwaoo_sensor_set_enable_retry(jwaoo_capacity_sensor_set_enable);
		} else if (jwaoo_sensor_set_enable_retry(fdc1004_set_enable)) {
			jwaoo_capacity_sensor_set_enable = fdc1004_set_enable;
			jwaoo_capacity_sensor_read_values = fdc1004_read_sensor_values;
		} else {
			jwaoo_capacity_sensor_read_values = jwaoo_sensor_read_values_dummy;
		}

		jwaoo_app_env.sensor_poll_enable = true;

		if (!ke_timer_active(JWAOO_TOY_SENSOR_POLL, TASK_JWAOO_TOY)) {
			ke_timer_set(JWAOO_TOY_SENSOR_POLL, TASK_JWAOO_TOY, 1);
		}
	} else {
		jwaoo_app_env.sensor_poll_enable = false;
		ke_timer_clear(JWAOO_TOY_SENSOR_POLL, TASK_JWAOO_TOY);

#if 0
		if (jwaoo_capacity_sensor_set_enable) {
			jwaoo_capacity_sensor_set_enable(false);
		}

		if (jwaoo_accel_sensor_set_enable) {
			jwaoo_accel_sensor_set_enable(false);
		}
#endif

		LDO_P3V3_CLOSE;
	}

	return true;
}

uint8_t jwaoo_sensor_poll(void)
{
	uint8_t buff[3 + FDC1004_DATA_BYTES + 1];

	if (jwaoo_app_env.sensor_accel_dead < 10) {
		if (jwaoo_accel_sensor_read_values(buff)) {
			jwaoo_app_env.sensor_accel_dead = 0;
		} else {
			jwaoo_app_env.sensor_accel_dead++;
			memset(buff, 0x00, 3);
		}
	} else {
		memset(buff, 0x00, 3);
	}

	if (jwaoo_app_env.sensor_capacity_dead < 10) {
		if (jwaoo_capacity_sensor_read_values(buff + 3)) {
			jwaoo_app_env.sensor_capacity_dead = 0;
		} else {
			jwaoo_app_env.sensor_capacity_dead++;
			memset(buff + 3, 0x00, FDC1004_DATA_BYTES);
		}
	} else {
		memset(buff + 3, 0x00, FDC1004_DATA_BYTES);
	}

	return jwaoo_toy_send_notify(JWAOO_TOY_ATTR_SENSOR_DATA, buff, 3 + FDC1004_DATA_BYTES);
}
