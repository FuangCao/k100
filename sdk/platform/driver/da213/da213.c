#include <stdint.h>
#include "da213.h"
#include "jwaoo_hw.h"

bool da213_set_enable(bool enable)
{
	int ret;
	uint8_t command;

	if (enable) {
		uint8_t id;

		ret = da213_read_register(DA213_REG_CHIPID, &id);
		if (ret < 0) {
			println("Failed to da213_read_register: %d", ret);
			return false;
		}

		println("da213: chip id = 0x%02x", id);

		if (id != 0x13) {
			return false;
		}

		command = 0x1E;
	} else {
		command = 0x9E;
	}

	ret = da213_write_register(DA213_REG_MODE_BW, command);
	if (ret < 0) {
		println("Failed to da213_write_register: %d", ret);
		return false;
	}

	return true;
}

bool da213_read_sensor_values(uint8_t values[6])
{
	int ret;

	ret = da213_read_data(DA213_REG_ACC_X_LSB, values, 6);
	if (ret < 0) {
		println("Failed to da213_read_data: %d", ret);
		return false;
	}

	return true;
}
