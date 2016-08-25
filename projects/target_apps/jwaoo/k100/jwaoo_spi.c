#include "jwaoo_spi.h"

void jwaoo_spi_set_enable(bool enable)
{
	if (enable) {
		GPIO_ConfigurePin(SPI_CS_GPIO_PORT, SPI_CS_GPIO_PIN, OUTPUT, PID_SPI_EN, true);
		GPIO_ConfigurePin(SPI_CLK_GPIO_PORT, SPI_CLK_GPIO_PIN, OUTPUT, PID_SPI_CLK, false);
		GPIO_ConfigurePin(SPI_DO_GPIO_PORT, SPI_DO_GPIO_PIN, OUTPUT, PID_SPI_DO, false);
		GPIO_ConfigurePin(SPI_DI_GPIO_PORT, SPI_DI_GPIO_PIN, INPUT, PID_SPI_DI, false);

		if (spi_flash_enable(SPI_CS_GPIO_PORT, SPI_CS_GPIO_PIN) < 0) {
			spi_flash_init(SPI_FLASH_DEFAULT_SIZE, SPI_FLASH_DEFAULT_PAGE);
		}
	} else {
		GPIO_ConfigurePin(SPI_CS_GPIO_PORT, SPI_CS_GPIO_PIN, OUTPUT, PID_GPIO, true);
		GPIO_ConfigurePin(SPI_CLK_GPIO_PORT, SPI_CLK_GPIO_PIN, INPUT, PID_GPIO, false);
		GPIO_ConfigurePin(SPI_DO_GPIO_PORT, SPI_DO_GPIO_PIN, INPUT, PID_GPIO, false);
		GPIO_ConfigurePin(SPI_DI_GPIO_PORT, SPI_DI_GPIO_PIN, INPUT, PID_GPIO, false);
	}
}
