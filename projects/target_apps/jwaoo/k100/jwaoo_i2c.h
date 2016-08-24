#pragma once

#include "jwaoo_hw.h"

enum
{
	JWAOO_I2C_SPEED_100K = 1,
	JWAOO_I2C_SPEED_400K,
};

enum
{
	JWAOO_I2C_ADDRESS_MODE_7BIT,
	JWAOO_I2C_ADDRESS_MODE_10BIT,
};

struct jwaoo_i2c_message
{
	uint8_t *data;
	uint8_t count;
	uint8_t read;
};

void jwaoo_i2c_init(uint8_t speed, uint8_t address_mode);
void jwaoo_i2c_release(void);

int jwaoo_i2c_transfer(uint8_t client, struct jwaoo_i2c_message *msgs, int count);
int jwaoo_i2c_read_data(uint8_t client, uint8_t addr, uint8_t *data, int size);
int jwaoo_i2c_write_data(uint8_t client, uint8_t addr, const uint8_t *data, int size);
int jwaoo_i2c_update_u8(uint8_t slave, uint8_t addr, uint8_t value, uint8_t mask);
int jwaoo_i2c_update_u16(uint8_t slave, uint8_t addr, uint16_t value, uint16_t mask);
int jwaoo_i2c_update_u32(uint8_t slave, uint8_t addr, uint32_t value, uint32_t mask);

static inline int jwaoo_i2c_read_u8(uint8_t slave, uint8_t addr, uint8_t *value)
{
	return jwaoo_i2c_read_data(slave, addr, value, sizeof(*value));
}

static inline int jwaoo_i2c_read_u16(uint8_t slave, uint8_t addr, uint16_t *value)
{
	return jwaoo_i2c_read_data(slave, addr, (uint8_t *) value, sizeof(*value));
}

static inline int jwaoo_i2c_read_u32(uint8_t slave, uint8_t addr, uint32_t *value)
{
	return jwaoo_i2c_read_data(slave, addr, (uint8_t *) value, sizeof(*value));
}

static inline int jwaoo_i2c_write_u8(uint8_t slave, uint8_t addr, uint8_t value)
{
	return jwaoo_i2c_write_data(slave, addr, &value, sizeof(value));
}

static inline int jwaoo_i2c_write_u16(uint8_t slave, uint8_t addr, uint16_t value)
{
	return jwaoo_i2c_write_data(slave, addr, (uint8_t *) &value, sizeof(value));
}

static inline int jwaoo_i2c_write_u32(uint8_t slave, uint8_t addr, uint32_t value)
{
	return jwaoo_i2c_write_data(slave, addr, (uint8_t *) &value, sizeof(value));
}

