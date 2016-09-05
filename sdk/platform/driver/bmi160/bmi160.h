#pragma once

#include <jwaoo_i2c.h>

#define BMI160_I2C_ADDRESS			0x68
#define BMI160_CHIP_ID				0xD1
#define BMI120_CHIP_ID				0xD3

enum {
	BMI160_REG_CMD = 0x7E,
	BMI160_REG_STEP_CONF_1 = 0x7B,
	BMI160_REG_STEP_CONF_0 = 0x7A,
	BMI160_REG_STEP_CNT_1 = 0x79,
	BMI160_REG_STEP_CNT_0 = 0x78,
	BMI160_REG_OFFSET_6 = 0x77,
	BMI160_REG_OFFSET_5 = 0x76,
	BMI160_REG_OFFSET_4 = 0x75,
	BMI160_REG_OFFSET_3 = 0x74,
	BMI160_REG_OFFSET_2 = 0x73,
	BMI160_REG_OFFSET_1 = 0x72,
	BMI160_REG_OFFSET_0 = 0x71,
	BMI160_REG_NV_CONF = 0x70,
	BMI160_REG_SELF_TEST = 0x6D,
	BMI160_REG_PMU_TRIGGER = 0x6C,
	BMI160_REG_IF_CONF = 0x6B,
	BMI160_REG_CONF = 0x6A,
	BMI160_REG_FOC_CONF = 0x69,
	BMI160_REG_INT_FLAT_1 = 0x68,
	BMI160_REG_INT_FLAT_0 = 0x67,
	BMI160_REG_INT_ORIENT_1 = 0x66,
	BMI160_REG_INT_ORIENT_0 = 0x65,
	BMI160_REG_INT_TAP_1 = 0x64,
	BMI160_REG_INT_TAP_0 = 0x63,
	BMI160_REG_INT_MOTION_3 = 0x62,
	BMI160_REG_INT_MOTION_2 = 0x61,
	BMI160_REG_INT_MOTION_1 = 0x60,
	BMI160_REG_INT_MOTION_0 = 0x5F,
	BMI160_REG_INT_LOWHIGH_4 = 0x5E,
	BMI160_REG_INT_LOWHIGH_3 = 0x5D,
	BMI160_REG_INT_LOWHIGH_2 = 0x5C,
	BMI160_REG_INT_LOWHIGH_1 = 0x5B,
	BMI160_REG_INT_LOWHIGH_0 = 0x5A,
	BMI160_REG_INT_DATA_1 = 0x59,
	BMI160_REG_INT_DATA_0 = 0x58,
	BMI160_REG_INT_MAP_2 = 0x57,
	BMI160_REG_INT_MAP_1 = 0x56,
	BMI160_REG_INT_MAP_0 = 0x55,
	BMI160_REG_INT_LATCH = 0x54,
	BMI160_REG_INT_OUT_CTRL = 0x53,
	BMI160_REG_INT_EN_2 = 0x52,
	BMI160_REG_INT_EN_1 = 0x51,
	BMI160_REG_INT_EN_0 = 0x50,
	BMI160_REG_MAG_IF_4 = 0x4F,
	BMI160_REG_MAG_IF_3 = 0x4E,
	BMI160_REG_MAG_IF_2 = 0x4D,
	BMI160_REG_MAG_IF_1 = 0x4C,
	BMI160_REG_MAG_IF_0 = 0x4B,
	BMI160_REG_FIFO_CONFIG_1 = 0x47,
	BMI160_REG_FIFO_CONFIG_0 = 0x46,
	BMI160_REG_FIFO_DOWNS = 0x45,
	BMI160_REG_MAG_CONF = 0x44,
	BMI160_REG_GYR_RANGE = 0x43,
	BMI160_REG_GYR_CONF = 0x42,
	BMI160_REG_ACC_RANGE = 0x41,
	BMI160_REG_ACC_CONF = 0x40,
	BMI160_REG_FIFO_DATA = 0x24,
	BMI160_REG_FIFO_LENGTH_1 = 0x23,
	BMI160_REG_FIFO_LENGTH_0 = 0x22,
	BMI160_REG_TEMPERATURE_1 = 0x21,
	BMI160_REG_TEMPERATURE_0 = 0x20,
	BMI160_REG_INT_STATUS_3 = 0x1F,
	BMI160_REG_INT_STATUS_2 = 0x1E,
	BMI160_REG_INT_STATUS_1 = 0x1D,
	BMI160_REG_INT_STATUS_0 = 0x1C,
	BMI160_REG_STATUS = 0x1B,
	BMI160_REG_SENSORTIME_2 = 0x1A,
	BMI160_REG_SENSORTIME_1 = 0x19,
	BMI160_REG_SENSORTIME_0 = 0x18,
	BMI160_REG_DATA_19 = 0x17,
	BMI160_REG_DATA_18 = 0x16,
	BMI160_REG_DATA_ACC_Z = 0x16,
	BMI160_REG_DATA_17 = 0x15,
	BMI160_REG_DATA_16 = 0x14,
	BMI160_REG_DATA_ACC_Y = 0x14,
	BMI160_REG_DATA_15 = 0x13,
	BMI160_REG_DATA_14 = 0x12,
	BMI160_REG_DATA_ACC_X = 0x12,
	BMI160_REG_DATA_ACC = 0x12,
	BMI160_REG_DATA_13 = 0x11,
	BMI160_REG_DATA_12 = 0x10,
	BMI160_REG_DATA_GYR_Z = 0x10,
	BMI160_REG_DATA_11 = 0x0F,
	BMI160_REG_DATA_10 = 0x0E,
	BMI160_REG_DATA_GYR_Y = 0x0E,
	BMI160_REG_DATA_9 = 0x0D,
	BMI160_REG_DATA_8 = 0x0C,
	BMI160_REG_DATA_GYR_X = 0x0C,
	BMI160_REG_DATA_GYR = 0x0C,
	BMI160_REG_DATA_7 = 0x0B,
	BMI160_REG_DATA_6 = 0x0A,
	BMI160_REG_DATA_RHALL = 0x0A,
	BMI160_REG_DATA_5 = 0x09,
	BMI160_REG_DATA_4 = 0x08,
	BMI160_REG_DATA_MAG_Z = 0x08,
	BMI160_REG_DATA_3 = 0x07,
	BMI160_REG_DATA_2 = 0x06,
	BMI160_REG_DATA_MAG_Y = 0x06,
	BMI160_REG_DATA_1 = 0x05,
	BMI160_REG_DATA_0 = 0x04,
	BMI160_REG_DATA_MAG_X = 0x04,
	BMI160_REG_DATA_MAG = 0x04,
	BMI160_REG_PMU_STATUS = 0x03,
	BMI160_REG_ERR_REG = 0x02,
	BMI160_REG_CHIP_ID = 0x00,
};

bool bmi160_set_enable(bool enable);
bool bmi160_read_sensor_values(uint8_t values[3]);

static inline int bmi160_read_data(uint8_t addr, uint8_t *data, int size)
{
	return jwaoo_i2c_read_data(BMI160_I2C_ADDRESS, addr, data, size);
}

static inline int bmi160_write_data(uint8_t addr, const uint8_t *data, int size)
{
	return jwaoo_i2c_write_data(BMI160_I2C_ADDRESS, addr, data, size);
}

static inline int bmi160_read_register(uint8_t addr, uint8_t *value)
{
	return bmi160_read_data(addr, value, 1);
}

static inline int bmi160_write_register(uint8_t addr, uint8_t value)
{
	return bmi160_write_data(addr, &value, 1);
}
