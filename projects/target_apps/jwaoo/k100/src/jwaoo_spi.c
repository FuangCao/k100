#include "jwaoo_spi.h"
#include "co_math.h"

struct jwaoo_partition_user_data jwaoo_user_data;
struct jwaoo_partition_device_data jwaoo_device_data;
struct jwaoo_partition_factory_data jwaoo_factory_data;

bool jwaoo_spi_partition_erase(uint32_t addr, uint32_t size)
{
	while (true) {
		if (spi_flash_block_erase(addr, SECTOR_ERASE) != ERR_OK) {
			return false;
		}

		if (size <= JWAOO_SPI_SECTOR_SIZE) {
			break;
		}

		addr += JWAOO_SPI_SECTOR_SIZE;
		size -= JWAOO_SPI_SECTOR_SIZE;
	}

	return true;
}

bool jwaoo_spi_partition_write(uint32_t addr, const struct jwaoo_partition_desc *part, uint32_t size)
{
	if (jwaoo_spi_partition_erase(addr, size)) {
		return spi_flash_write_data((uint8_t *) part, addr, size) == size;
	}

	return false;
}

bool jwaoo_spi_partition_read(uint32_t addr, struct jwaoo_partition_desc *part, uint32_t size)
{
	if (spi_flash_read_data((uint8_t *) part, addr, size) != size) {
		return false;
	}

	if (part->magic != JWAOO_SPI_MAGIC) {
		memset(part, 0x00, size);
		part->magic = JWAOO_SPI_MAGIC;
	}

	return true;
}

void jwaoo_spi_set_enable(bool enable)
{
	if (enable) {
		GPIO_ConfigurePin(SPI_CS_GPIO_PORT, SPI_CS_GPIO_PIN, OUTPUT, PID_SPI_EN, true);
		GPIO_ConfigurePin(SPI_CLK_GPIO_PORT, SPI_CLK_GPIO_PIN, OUTPUT, PID_SPI_CLK, false);
		GPIO_ConfigurePin(SPI_DO_GPIO_PORT, SPI_DO_GPIO_PIN, OUTPUT, PID_SPI_DO, false);
		GPIO_ConfigurePin(SPI_DI_GPIO_PORT, SPI_DI_GPIO_PIN, INPUT, PID_SPI_DI, false);

		if (spi_flash_enable(SPI_CS_GPIO_PORT, SPI_CS_GPIO_PIN) < 0) {
			spi_flash_init(JWAOO_SPI_FLASH_DEFAULT_SIZE, JWAOO_SPI_FLASH_DEFAULT_PAGE);
		}
	} else {
		GPIO_ConfigurePin(SPI_CS_GPIO_PORT, SPI_CS_GPIO_PIN, OUTPUT, PID_GPIO, true);
		GPIO_ConfigurePin(SPI_CLK_GPIO_PORT, SPI_CLK_GPIO_PIN, INPUT, PID_GPIO, false);
		GPIO_ConfigurePin(SPI_DO_GPIO_PORT, SPI_DO_GPIO_PIN, INPUT, PID_GPIO, false);
		GPIO_ConfigurePin(SPI_DI_GPIO_PORT, SPI_DI_GPIO_PIN, INPUT, PID_GPIO, false);
	}
}

void jwaoo_spi_load_data(void)
{
	jwaoo_spi_read_user_data();
	jwaoo_spi_read_device_data();
	jwaoo_spi_read_factory_data();
}

uint8_t jwaoo_spi_calculate_crc(const uint8_t *mem, uint32_t size, uint8_t crc)
{
	const uint8_t *mem_end;

	for (mem_end = mem + size; mem < mem_end; mem++) {
		crc ^= *mem;
	}

	return crc;
}

bool jwaoo_spi_flash_check_crc(uint32_t addr, uint32_t size, uint8_t crc_raw)
{
	uint8_t crc = 0xFF;

	while (size > 0) {
		int ret;
		int length;
		uint8_t buff[128];

		length = (size > sizeof(buff) ? sizeof(buff) : size);

		ret = spi_flash_read_data(buff, addr, length);
		if (ret != length) {
			println("Failed to spi_flash_read_data: %d", ret);
			return false;
		}

		crc = jwaoo_spi_calculate_crc(buff, length, crc);
		addr += length;
		size -= length;
	}

	if (crc != crc_raw) {
		println("crc not match: 0x%02x != 0x%02x", crc, crc_raw);
		return false;
	}

	return true;
}

static bool jwaoo_spi_flash_copy_safe(uint32_t rdaddr, uint32_t wraddr, uint32_t size, uint8_t crc_raw)
{
	uint8_t crc_read = 0xFF;
	uint8_t crc_write = 0xFF;

	println("spi_flash_block_erase: 0x%04x", wraddr);

	if (spi_flash_block_erase(wraddr, JWAOO_SPI_CODE_ERASE_MODE) != ERR_OK) {
		println("Failed to spi_flash_block_erase");
		return false;
	}

	println("%s: 0x%04x [%d]=> 0x%04x", __FUNCTION__, rdaddr, size, wraddr);

	while (size > 0) {
		int ret;
		int length;
		static uint8_t buff[128];

		length = (size > sizeof(buff) ? sizeof(buff) : size);

		ret = spi_flash_read_data(buff, rdaddr, length);
		if (ret != length) {
			println("Failed to spi_flash_read_data: %d", ret);
			return false;
		}

		crc_read = jwaoo_spi_calculate_crc(buff, length, crc_read);

		ret = spi_flash_write_data(buff, wraddr, length);
		if (ret != length) {
			println("Failed to spi_flash_write_data: %d", ret);
			return false;
		}

		ret = spi_flash_read_data(buff, wraddr, length);
		if (ret != length) {
			println("Failed to spi_flash_read_data: %d", ret);
			return false;
		}

		crc_write = jwaoo_spi_calculate_crc(buff, length, crc_write);

		rdaddr += length;
		wraddr += length;
		size -= length;
	}

	if (crc_read != crc_raw) {
		println("read crc not match: 0x%02x != 0x%02x", crc_read, crc_raw);
		return false;
	}

	if (crc_write != crc_raw) {
		println("write crc not match: 0x%02x != 0x%02x", crc_write, crc_raw);
		return false;
	}

	println("%s successfull", __FUNCTION__);

	return true;
}

bool jwaoo_spi_flash_copy(uint32_t rdaddr, uint32_t wraddr, uint32_t size, uint8_t crc_raw)
{
	bool success;

#if USE_WDOG
	wdg_freeze();
#endif

	success = jwaoo_spi_flash_copy_safe(rdaddr, wraddr, size, crc_raw);

#if USE_WDOG
	wdg_resume();
#endif

	return success;
}

bool jwaoo_spi_write_bd_addr(const uint8_t bd_addr[6])
{
	memcpy(jwaoo_device_data.bd_addr, bd_addr, 6);

	return jwaoo_spi_write_device_data();
}

static bool jwaoo_is_invalid_bd_addr(const uint8_t bd_addr[6])
{
	const uint8_t *p, *p_end;

	for (p = bd_addr, p_end = p + 6; p < p_end; p++) {
		if (*p != 0x00 && *p != 0xFF) {
			return false;
		}
	}

	return true;
}

bool jwaoo_spi_read_bd_addr(uint8_t bd_addr[6])
{
	uint8_t *mac = jwaoo_device_data.bd_addr;

	if (jwaoo_is_invalid_bd_addr(mac)) {
		uint8_t count;

		mac = jwaoo_device_data.bd_addr_rand;

		for (count = 0; jwaoo_is_invalid_bd_addr(mac); count++) {
			uint8_t *p, *p_end;

			if (count > 100) {
				return false;
			}

			for (p = mac, p_end = p + 6; p < p_end; p++) {
				*p = rand() & 0xFF;
			}
		}

		if (count > 0) {
			jwaoo_spi_write_device_data();
		}
	}

	memcpy(bd_addr, mac, 6);

	return true;
}
