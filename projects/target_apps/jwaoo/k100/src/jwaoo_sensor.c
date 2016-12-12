#include "jwaoo_toy_task.h"
#include "jwaoo_sensor.h"
#include "jwaoo_app.h"
#include "jwaoo_toy.h"
#include "da213.h"

static bool jwaoo_sensor_read_values_dummy(uint8_t values[6])
{
	return false;
}

static bool (*jwaoo_accel_sensor_set_enable)(bool enable);
static bool (*jwaoo_accel_sensor_read_values)(uint8_t values[6]) = jwaoo_sensor_read_values_dummy;

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
#ifdef LDO_P3V3_OPEN
		LDO_P3V3_OPEN;
#endif

		if (jwaoo_accel_sensor_set_enable) {
			jwaoo_sensor_set_enable_retry(jwaoo_accel_sensor_set_enable);
		} else if (jwaoo_sensor_set_enable_retry(da213_set_enable)) {
			jwaoo_accel_sensor_set_enable = da213_set_enable;
			jwaoo_accel_sensor_read_values = da213_read_sensor_values;
		} else {
			jwaoo_accel_sensor_read_values = jwaoo_sensor_read_values_dummy;
		}

		jwaoo_app_env.sensor_poll_enable = true;

		if (!ke_timer_active(JWAOO_TOY_SENSOR_POLL, TASK_JWAOO_TOY)) {
			ke_timer_set(JWAOO_TOY_SENSOR_POLL, TASK_JWAOO_TOY, 1);
		}
	} else {
		jwaoo_app_env.sensor_poll_enable = false;
		ke_timer_clear(JWAOO_TOY_SENSOR_POLL, TASK_JWAOO_TOY);

#ifdef LDO_P3V3_CLOSE
		LDO_P3V3_CLOSE;
#else
		if (jwaoo_accel_sensor_set_enable) {
			jwaoo_accel_sensor_set_enable(false);
		}
#endif
	}

	return true;
}

uint8_t jwaoo_sensor_poll(void)
{
	uint8_t buff[6];

	if (jwaoo_app_env.sensor_accel_dead < 10) {
		if (jwaoo_accel_sensor_read_values(buff)) {
			jwaoo_app_env.sensor_accel_dead = 0;
			return jwaoo_toy_send_notify(JWAOO_TOY_ATTR_SENSOR_DATA, buff, sizeof(buff));
		} else {
			jwaoo_app_env.sensor_accel_dead++;
		}
	}

	return 0;
}
