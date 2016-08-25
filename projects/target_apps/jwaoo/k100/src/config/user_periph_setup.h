#pragma once

/**
 ****************************************************************************************
 *
 * @file user_periph_setup.h
 *
 * @brief Peripherals setup header file.
 *
 * Copyright (C) 2015. Dialog Semiconductor Ltd, unpublished work. This computer
 * program includes Confidential, Proprietary Information and is a Trade Secret of
 * Dialog Semiconductor Ltd.  All use, disclosure, and/or reproduction is prohibited
 * unless authorized in writing. All Rights Reserved.
 *
 * <bluetooth.support@diasemi.com> and contributors.
 *
 ****************************************************************************************
 */

#include "rwip_config.h"
#include "global_io.h"
#include "arch.h"
#include "da1458x_periph_setup.h"
#include "i2c_eeprom.h"

#define PWM_LEVEL_MAX			100
#define KEY_ACTIVE_LOW			0

#define MOTO_GPIO_PORT			GPIO_PORT_2
#define MOTO_GPIO_PIN			GPIO_PIN_0
#define MOTO_GPIO_RESERVE		RESERVE_GPIO(MOTO, MOTO_GPIO_PORT, MOTO_GPIO_PIN, PID_GPIO)
#define MOTO_GPIO_CONFIG 		GPIO_ConfigurePin(MOTO_GPIO_PORT, MOTO_GPIO_PIN, OUTPUT, PID_GPIO, false)
#define MOTO_OPEN				GPIO_SetActive(MOTO_GPIO_PORT, MOTO_GPIO_PIN)
#define MOTO_CLOSE				GPIO_SetInactive(MOTO_GPIO_PORT, MOTO_GPIO_PIN)

#define LED1_GPIO_PORT			GPIO_PORT_0
#define LED1_GPIO_PIN			GPIO_PIN_7
#define LED1_GPIO_RESERVE		RESERVE_GPIO(LED1, LED1_GPIO_PORT, LED1_GPIO_PIN, PID_GPIO)
#define LED1_GPIO_CONFIG 		GPIO_ConfigurePin(LED1_GPIO_PORT, LED1_GPIO_PIN, OUTPUT, PID_GPIO, false)
#define LED1_OPEN				GPIO_SetActive(LED1_GPIO_PORT, LED1_GPIO_PIN)
#define LED1_CLOSE				GPIO_SetInactive(LED1_GPIO_PORT, LED1_GPIO_PIN)

#define LED2_GPIO_PORT			GPIO_PORT_2
#define LED2_GPIO_PIN			GPIO_PIN_9
#define LED2_GPIO_RESERVE		RESERVE_GPIO(LED2, LED2_GPIO_PORT, LED2_GPIO_PIN, PID_GPIO)
#define LED2_GPIO_CONFIG 		GPIO_ConfigurePin(LED2_GPIO_PORT, LED2_GPIO_PIN, OUTPUT, PID_GPIO, false)
#define LED2_OPEN				GPIO_SetActive(LED2_GPIO_PORT, LED2_GPIO_PIN)
#define LED2_CLOSE				GPIO_SetInactive(LED2_GPIO_PORT, LED2_GPIO_PIN)

#define BATT_ADC_GPIO_PORT		GPIO_PORT_0
#define BATT_ADC_GPIO_PIN		GPIO_PIN_1
#define BATT_ADC_GPIO_RESERVE	RESERVE_GPIO(BATT_ADC, BATT_ADC_GPIO_PORT, BATT_ADC_GPIO_PIN, PID_ADC)
#define BATT_ADC_GPIO_CONFIG 	GPIO_ConfigurePin(BATT_ADC_GPIO_PORT, BATT_ADC_GPIO_PIN, INPUT, PID_ADC, true)

#define CHG_STAT_GPIO_PORT		GPIO_PORT_1
#define CHG_STAT_GPIO_PIN		GPIO_PIN_1
#define CHG_STAT_GPIO_IRQ		GPIO4_IRQn
#define CHG_STAT_GPIO_RESERVE	RESERVE_GPIO(CHG_STAT, CHG_STAT_GPIO_PORT, CHG_STAT_GPIO_PIN, PID_GPIO)
#define CHG_STAT_GPIO_CONFIG 	GPIO_ConfigurePin(CHG_STAT_GPIO_PORT, CHG_STAT_GPIO_PIN, INPUT, PID_GPIO, true)
#define CHG_STAT_GPIO_GET		GPIO_GetPinStatus(CHG_STAT_GPIO_PORT, CHG_STAT_GPIO_PIN)

#define LDO_P3V3_GPIO_PORT		GPIO_PORT_0
#define LDO_P3V3_GPIO_PIN		GPIO_PIN_2
#define LDO_P3V3_GPIO_RESERVE	RESERVE_GPIO(LDO_P3V3, LDO_P3V3_GPIO_PORT, LDO_P3V3_GPIO_PIN, PID_GPIO)
#define LDO_P3V3_GPIO_CONFIG 	GPIO_ConfigurePin(LDO_P3V3_GPIO_PORT, LDO_P3V3_GPIO_PIN, OUTPUT, PID_GPIO, false)
#define LDO_P3V3_OPEN			GPIO_SetActive(LDO_P3V3_GPIO_PORT, LDO_P3V3_GPIO_PIN)
#define LDO_P3V3_CLOSE			GPIO_SetInactive(LDO_P3V3_GPIO_PORT, LDO_P3V3_GPIO_PIN)

#define KEY_GPIO_RESERVE(index) \
	RESERVE_GPIO(KEY##index, KEY##index##_GPIO_PORT, KEY##index##_GPIO_PIN, PID_GPIO)

#if KEY_ACTIVE_LOW
#define KEY_GPIO_CONFIG(index) \
	GPIO_ConfigurePin(KEY##index##_GPIO_PORT, KEY##index##_GPIO_PIN, INPUT_PULLUP, PID_GPIO, true)

#define KEY_IS_ACTIVE(port, pin) \
	(GPIO_GetPinStatus(port, pin) == 0)
#else
#define KEY_GPIO_CONFIG(index) \
	GPIO_ConfigurePin(KEY##index##_GPIO_PORT, KEY##index##_GPIO_PIN, INPUT_PULLDOWN, PID_GPIO, true)

#define KEY_IS_ACTIVE(port, pin) \
	GPIO_GetPinStatus(port, pin)
#endif

#define KEY1_GPIO_PORT			GPIO_PORT_2
#define KEY1_GPIO_PIN			GPIO_PIN_1
#define KEY1_GPIO_IRQ			GPIO0_IRQn

#define KEY2_GPIO_PORT			GPIO_PORT_2
#define KEY2_GPIO_PIN			GPIO_PIN_2
#define KEY2_GPIO_IRQ			GPIO1_IRQn

#define KEY3_GPIO_PORT			GPIO_PORT_2
#define KEY3_GPIO_PIN			GPIO_PIN_3
#define KEY3_GPIO_IRQ			GPIO2_IRQn

#define KEY4_GPIO_PORT			GPIO_PORT_2
#define KEY4_GPIO_PIN			GPIO_PIN_4
#define KEY4_GPIO_IRQ			GPIO3_IRQn

/****************************************************************************************/
/* i2c eeprom configuration                                                             */
/****************************************************************************************/

#define I2C1_GPIO_PORT		GPIO_PORT_1
#define I2C1_SDA_GPIO_PIN	GPIO_PIN_2
#define I2C1_SCL_GPIO_PIN	GPIO_PIN_3

#define I2C2_GPIO_PORT		GPIO_PORT_0
#define I2C2_SDA_GPIO_PIN	GPIO_PIN_2
#define I2C2_SCL_GPIO_PIN	GPIO_PIN_7

#define I2C_EEPROM_SIZE		0x20000         // EEPROM size in bytes
#define I2C_EEPROM_PAGE		256             // EEPROM's page size in bytes
#define I2C_SPEED_MODE		I2C_FAST        // 1: standard mode (100 kbits/s), 2: fast mode (400 kbits/s)
#define I2C_ADDRESS_MODE	I2C_7BIT_ADDR   // 0: 7-bit addressing, 1: 10-bit addressing
#define I2C_ADDRESS_SIZE	I2C_2BYTES_ADDR // 0: 8-bit memory address, 1: 16-bit memory address, 3: 24-bit memory address

/****************************************************************************************/
/* SPI FLASH configuration                                                              */
/****************************************************************************************/

#define SPI_FLASH_DEFAULT_SIZE	0x40000    // SPI Flash memory size in bytes
#define SPI_FLASH_DEFAULT_PAGE	0x100
#define SPI_SECTOR_SIZE			4096
#define SPI_SECTOR_SIZE_MASK	((SPI_SECTOR_SIZE) - 1)

#define SPI_CODE_SIZE			KB(32)
#define SPI_PART_FRONT_CODE		0
#define SPI_PART_BACK_CODE		(SPI_PART_FRONT_CODE + SPI_CODE_SIZE)
#define SPI_PART_DEVICE_DATA	(SPI_PART_BACK_CODE + SPI_CODE_SIZE)

#define SPI_CS_GPIO_PORT		GPIO_PORT_0
#define SPI_CS_GPIO_PIN			GPIO_PIN_3

#define SPI_CLK_GPIO_PORT		GPIO_PORT_0
#define SPI_CLK_GPIO_PIN		GPIO_PIN_0

#define SPI_DO_GPIO_PORT		GPIO_PORT_0
#define SPI_DO_GPIO_PIN			GPIO_PIN_6

#define SPI_DI_GPIO_PORT		GPIO_PORT_0
#define SPI_DI_GPIO_PIN			GPIO_PIN_5

/****************************************************************************************/
/* uart pin configuration                                                               */
/****************************************************************************************/

#define UART_TX_GPIO_PORT		GPIO_PORT_0
#define UART_TX_GPIO_PIN		GPIO_PIN_4

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Enable pad's and peripheral clocks assuming that peripherals' power domain
 * is down. The Uart and SPI clocks are set.
 * @return void
 ****************************************************************************************
 */
void periph_init(void);

/**
 ****************************************************************************************
 * @brief Map port pins. The Uart and SPI port pins and GPIO ports are mapped.
 * @return void
 ****************************************************************************************
 */
void set_pad_functions(void);

/**
 ****************************************************************************************
 * @brief Each application reserves its own GPIOs here.
 * @return void
 ****************************************************************************************
 */
void GPIO_reservations(void);
