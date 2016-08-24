#include "jwaoo_spi.h"

void jwaoo_spi_init(void)
{
	if (spi_flash_enable(SPI_CS_GPIO_PORT, SPI_CS_GPIO_PIN) < 0)
	{
		spi_flash_init(SPI_FLASH_DEFAULT_SIZE, SPI_FLASH_DEFAULT_PAGE);
	}
}
