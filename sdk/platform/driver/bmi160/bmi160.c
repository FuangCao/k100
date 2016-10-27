#include <stdint.h>
#include "bmi160.h"
#include "jwaoo_hw.h"

bool bmi160_set_enable(bool enable)
{
	int ret;
	uint8_t command;

	if (enable) {
		uint8_t id;

		ret = bmi160_read_register(BMI160_REG_CHIP_ID, &id);
		if (ret < 0) {
			println("Failed to bmi160_read_register: %d", ret);
			return false;
		}

		println("bmi160: chip id = 0x%02x", id);

		switch (id) {
		case BMI160_CHIP_ID:
			println("BMI160 found");
			break;

		case BMI120_CHIP_ID:
			println("BMI120 found");
			break;

		default:
			println("Invalid chip id");
			return false;
		}

		command = 0x11;
	} else {
		command = 0x10;
	}

	ret = bmi160_write_register(BMI160_REG_CMD, command);
	if (ret < 0) {
		println("Failed to bmi160_write_register: %d", ret);
		return false;
	}

	return true;
}

bool bmi160_read_sensor_values(uint8_t values[6])
{
	int ret;

	ret = bmi160_read_data(BMI160_REG_DATA_ACC, values, 6);
	if (ret < 0) {
		println("Failed to bmi160_read_data: %d", ret);
		return false;
	}

	return true;
}
