#pragma once

#include <jwaoo_i2c.h>

#define DA213_I2C_ADDRESS			0x27

enum {
	DA213_REG_SPI_CONFIG = 0x00,
	DA213_REG_CHIPID = 0x01,
	DA213_REG_ACC_X_LSB = 0x02,
	DA213_REG_ACC_X_MSB = 0x03,
	DA213_REG_ACC_Y_LSB = 0x04,
	DA213_REG_ACC_Y_MSB = 0x05,
	DA213_REG_ACC_Z_LSB = 0x06,
	DA213_REG_ACC_Z_MSB = 0x07,
	DA213_REG_MOTION_FLAG = 0x09,
	DA213_REG_NEWDATA_FLAG = 0x0A,
	DA213_REG_TAP_ACTIVE_STATUS = 0x0B,
	DA213_REG_ORIENT_STATUS = 0x0C,
	DA213_REG_RESOLUTION_RANGE = 0x0F,
	DA213_REG_ODR_AXIS = 0x10,
	DA213_REG_MODE_BW = 0x11,
	DA213_REG_SWAP_POLARITY = 0x12,
	DA213_REG_INT_SET1 = 0x16,
	DA213_REG_INT_SET2 = 0x17,
	DA213_REG_INT_MAP1 = 0x19,
	DA213_REG_INT_MAP2 = 0x1A,
	DA213_REG_INT_MAP3 = 0x1B,
	DA213_REG_INT_CONFIG = 0x20,
	DA213_REG_INT_LTACH = 0x21,
	DA213_REG_FREEFALL_DUR = 0x22,
	DA213_REG_FREEFALL_THS = 0x23,
	DA213_REG_FREEFALL_HYST = 0x24,
	DA213_REG_ACTIVE_DUR = 0x27,
	DA213_REG_ACTIVE_THS = 0x28,
	DA213_REG_TAP_DUR = 0x2A,
	DA213_REG_TAP_THS = 0x2B,
	DA213_REG_ORIENT_HYST = 0x2C,
	DA213_REG_Z_BLOCK = 0x2D,
};

bool da213_set_enable(bool enable);
bool da213_read_sensor_values(uint8_t values[6]);

static inline int da213_read_data(uint8_t addr, uint8_t *data, int size)
{
	return jwaoo_i2c_read_data(DA213_I2C_ADDRESS, addr, data, size);
}

static inline int da213_write_data(uint8_t addr, const uint8_t *data, int size)
{
	return jwaoo_i2c_write_data(DA213_I2C_ADDRESS, addr, data, size);
}

static inline int da213_read_register(uint8_t addr, uint8_t *value)
{
	return da213_read_data(addr, value, 1);
}

static inline int da213_write_register(uint8_t addr, uint8_t value)
{
	return da213_write_data(addr, &value, 1);
}
