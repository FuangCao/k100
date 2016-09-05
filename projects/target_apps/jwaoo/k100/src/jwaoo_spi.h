#pragma once

#include "jwaoo_hw.h"
#include "spi.h"
#include "spi_flash.h"

#ifndef KB
#define KB(value)						((value) << 10)
#endif

#define JWAOO_SPI_MAGIC					0x11223344

#define JWAOO_SPI_FLASH_DEFAULT_SIZE	0x40000    // SPI Flash memory size in bytes
#define JWAOO_SPI_FLASH_DEFAULT_PAGE	0x100
#define JWAOO_SPI_SECTOR_SIZE			4096

#define JWAOO_SPI_SECTOR_SIZE_MASK \
	((JWAOO_SPI_SECTOR_SIZE) - 1)

#define JWAOO_SPI_SECTOR_ALIGN(size) \
	(((size) + JWAOO_SPI_SECTOR_SIZE_MASK) & (~JWAOO_SPI_SECTOR_SIZE_MASK))

#define JWAOO_SPI_PART_SIZE(type) \
	JWAOO_SPI_SECTOR_ALIGN(sizeof(struct type))

#define JWAOO_SPI_CODE_SIZE				KB(32)
#define JWAOO_SPI_DEVICE_DATA_SIZE		JWAOO_SPI_PART_SIZE(jwaoo_partition_device_data)
#define JWAOO_SPI_FACTORY_DATA_SIZE		JWAOO_SPI_PART_SIZE(jwaoo_partition_factory_data)
#define JWAOO_SPI_USER_DATA_SIZE		JWAOO_SPI_PART_SIZE(jwaoo_partition_user_data)

#define JWAOO_SPI_PART_FRONT_CODE		0
#define JWAOO_SPI_PART_BACK_CODE		(JWAOO_SPI_PART_FRONT_CODE + JWAOO_SPI_CODE_SIZE)
#define JWAOO_SPI_PART_DEVICE_DATA		(JWAOO_SPI_PART_BACK_CODE + JWAOO_SPI_CODE_SIZE)
#define JWAOO_SPI_PART_FACTORY_DATA		(JWAOO_SPI_PART_DEVICE_DATA + JWAOO_SPI_CODE_SIZE)
#define JWAOO_SPI_PART_USER_DATA		(JWAOO_SPI_PART_FACTORY_DATA + JWAOO_SPI_CODE_SIZE)

#if JWAOO_SPI_CODE_SIZE > KB(32)
#define JWAOO_SPI_CODE_ERASE_MODE		BLOCK_ERASE_64
#else
#define JWAOO_SPI_CODE_ERASE_MODE		BLOCK_ERASE_32
#endif

struct jwaoo_partition_desc {
	uint32_t magic;
};

struct jwaoo_partition_device_data {
	struct jwaoo_partition_desc desc;
	uint8_t bd_addr[6];
	uint8_t bd_addr_rand[6];
};

struct jwaoo_partition_factory_data {
	struct jwaoo_partition_desc desc;
	uint8_t test_result[16];
};

struct jwaoo_partition_user_data {
	struct jwaoo_partition_desc desc;
	bool auto_suspend_enable;
	uint16_t auto_suspend_time;
};

extern uint32_t spi_flash_jedec_id;
extern uint32_t spi_flash_size;
extern uint32_t spi_flash_page_size;

extern struct jwaoo_partition_user_data jwaoo_user_data;
extern struct jwaoo_partition_device_data jwaoo_device_data;
extern struct jwaoo_partition_factory_data jwaoo_factory_data;

bool jwaoo_spi_partition_erase(uint32_t addr, uint32_t size);
bool jwaoo_spi_partition_write(uint32_t addr, const struct jwaoo_partition_desc *part, uint32_t size);
bool jwaoo_spi_partition_read(uint32_t addr, struct jwaoo_partition_desc *part, uint32_t size);

void jwaoo_spi_set_enable(bool enable);
void jwaoo_spi_load_data(void);

bool jwaoo_spi_write_bd_addr(const uint8_t bd_addr[6]);
bool jwaoo_spi_read_bd_addr(uint8_t bd_addr[6]);
uint8_t jwaoo_spi_calculate_crc(const uint8_t *mem, uint32_t size, uint8_t crc);
bool jwaoo_spi_flash_check_crc(uint32_t addr, uint32_t size, uint8_t crc_raw);
bool jwaoo_spi_flash_copy(uint32_t rdaddr, uint32_t wraddr, uint32_t size, uint8_t crc_raw);

static inline bool jwaoo_spi_read_device_data(void)
{
	return jwaoo_spi_partition_read(JWAOO_SPI_PART_DEVICE_DATA, &jwaoo_device_data.desc, sizeof(jwaoo_device_data));
}

static inline bool jwaoo_spi_write_device_data(void)
{
	return jwaoo_spi_partition_write(JWAOO_SPI_PART_DEVICE_DATA, &jwaoo_device_data.desc, sizeof(jwaoo_device_data));
}

static inline bool jwaoo_spi_read_factory_data(void)
{
	return jwaoo_spi_partition_read(JWAOO_SPI_PART_FACTORY_DATA, &jwaoo_factory_data.desc, sizeof(jwaoo_factory_data));
}

static inline bool jwaoo_spi_write_factory_data(void)
{
	return jwaoo_spi_partition_write(JWAOO_SPI_PART_FACTORY_DATA, &jwaoo_factory_data.desc, sizeof(jwaoo_factory_data));
}

static inline bool jwaoo_spi_read_user_data(void)
{
	return jwaoo_spi_partition_read(JWAOO_SPI_PART_USER_DATA, &jwaoo_user_data.desc, sizeof(jwaoo_user_data));
}

static inline bool jwaoo_spi_write_user_data(void)
{
	return jwaoo_spi_partition_write(JWAOO_SPI_PART_USER_DATA, &jwaoo_user_data.desc, sizeof(jwaoo_user_data));
}

