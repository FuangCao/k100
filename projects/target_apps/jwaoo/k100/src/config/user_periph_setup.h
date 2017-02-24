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

#define JWAOO_BOARD_K101		1

#define USE_JWAOO_GPIO_ISR
#define KEY_ACTIVE_LOW			0
#define CHG_DET_ACTIVE_LOW		0
#define CHG_STAT_ACTIVE_LOW		1

#define GPIO_BLINK(port, pin) \
	do { \
		if (GPIO_GetPinStatus(port, pin)) { \
			GPIO_SetInactive(port, pin) ; \
		} else { \
			GPIO_SetActive(port, pin); \
		} \
	} while (0)

#define MOTO_GPIO_PORT			GPIO_PORT_2
#define MOTO_GPIO_PIN			GPIO_PIN_0
#define MOTO_GPIO_RESERVE		RESERVE_GPIO(MOTO, MOTO_GPIO_PORT, MOTO_GPIO_PIN, PID_GPIO)
#define MOTO_GPIO_CONFIG 		GPIO_ConfigurePin(MOTO_GPIO_PORT, MOTO_GPIO_PIN, OUTPUT, PID_GPIO, false)
#define MOTO_OPEN				GPIO_SetActive(MOTO_GPIO_PORT, MOTO_GPIO_PIN)
#define MOTO_CLOSE				GPIO_SetInactive(MOTO_GPIO_PORT, MOTO_GPIO_PIN)

#define RESISTOR_PARALLEL		1
#define RESISTOR_GPIO_PORT		GPIO_PORT_0
#define RESISTOR_GPIO_PIN		GPIO_PIN_7
#define RESISTOR_GPIO_RESERVE	RESERVE_GPIO(RESISTOR, RESISTOR_GPIO_PORT, RESISTOR_GPIO_PIN, PID_GPIO)
#define RESISTOR_GPIO_CONFIG 	GPIO_ConfigurePin(RESISTOR_GPIO_PORT, RESISTOR_GPIO_PIN, OUTPUT, PID_GPIO, false)
#define RESISTOR_SET_HIGH		GPIO_SetActive(RESISTOR_GPIO_PORT, RESISTOR_GPIO_PIN)
#define RESISTOR_SET_LOW		GPIO_SetInactive(RESISTOR_GPIO_PORT, RESISTOR_GPIO_PIN)
#if RESISTOR_PARALLEL
#define RESISTOR_OPEN			RESISTOR_SET_HIGH
#define RESISTOR_CLOSE			RESISTOR_SET_LOW
#else
#define RESISTOR_OPEN			RESISTOR_SET_LOW
#define RESISTOR_CLOSE			RESISTOR_SET_HIGH
#endif

#define LIMIT_IC_GPIO_PORT		GPIO_PORT_2
#define LIMIT_IC_GPIO_PIN		GPIO_PIN_8
#define LIMIT_IC_GPIO_RESERVE	RESERVE_GPIO(LIMIT_IC, LIMIT_IC_GPIO_PORT, LIMIT_IC_GPIO_PIN, PID_GPIO)
#define LIMIT_IC_GPIO_CONFIG 	GPIO_ConfigurePin(LIMIT_IC_GPIO_PORT, LIMIT_IC_GPIO_PIN, OUTPUT, PID_GPIO, false)
#define LIMIT_IC_OPEN			GPIO_SetActive(LIMIT_IC_GPIO_PORT, LIMIT_IC_GPIO_PIN)
#define LIMIT_IC_CLOSE			GPIO_SetInactive(LIMIT_IC_GPIO_PORT, LIMIT_IC_GPIO_PIN)

#define BATT_LED_GPIO_PORT		GPIO_PORT_2
#define BATT_LED_GPIO_PIN		GPIO_PIN_9
#define BATT_LED_GPIO_RESERVE	RESERVE_GPIO(BATT_LED, BATT_LED_GPIO_PORT, BATT_LED_GPIO_PIN, PID_GPIO)
#define BATT_LED_GPIO_CONFIG 	GPIO_ConfigurePin(BATT_LED_GPIO_PORT, BATT_LED_GPIO_PIN, OUTPUT, PID_GPIO, false)
#define BATT_LED_OPEN			GPIO_SetActive(BATT_LED_GPIO_PORT, BATT_LED_GPIO_PIN)
#define BATT_LED_CLOSE			GPIO_SetInactive(BATT_LED_GPIO_PORT, BATT_LED_GPIO_PIN)
#define BATT_LED_BLINK			GPIO_BLINK(BATT_LED_GPIO_PORT, BATT_LED_GPIO_PIN)

#define BT_LED_GPIO_PORT		GPIO_PORT_0
#define BT_LED_GPIO_PIN			GPIO_PIN_7
#define BT_LED_GPIO_RESERVE		RESERVE_GPIO(BT_LED, BT_LED_GPIO_PORT, BT_LED_GPIO_PIN, PID_GPIO)
#define BT_LED_GPIO_CONFIG 		GPIO_ConfigurePin(BT_LED_GPIO_PORT, BT_LED_GPIO_PIN, OUTPUT, PID_GPIO, false)
#define BT_LED_OPEN				GPIO_SetActive(BT_LED_GPIO_PORT, BT_LED_GPIO_PIN)
#define BT_LED_CLOSE			GPIO_SetInactive(BT_LED_GPIO_PORT, BT_LED_GPIO_PIN)
#define BT_LED_BLINK			GPIO_BLINK(BT_LED_GPIO_PORT, BT_LED_GPIO_PIN)
#define BT_LED_STATE			GPIO_GetPinStatus(BT_LED_GPIO_PORT, BT_LED_GPIO_PIN)

#define BATT_ADC_GPIO_PORT		GPIO_PORT_0
#define BATT_ADC_GPIO_PIN		GPIO_PIN_1
#define BATT_ADC_GPIO_RESERVE	RESERVE_GPIO(BATT_ADC, BATT_ADC_GPIO_PORT, BATT_ADC_GPIO_PIN, PID_ADC)
#define BATT_ADC_GPIO_CONFIG 	GPIO_ConfigurePin(BATT_ADC_GPIO_PORT, BATT_ADC_GPIO_PIN, INPUT, PID_ADC, true)

#define CHG_DET_GPIO_PORT		GPIO_PORT_1
#define CHG_DET_GPIO_PIN		GPIO_PIN_0
#define CHG_DET_GPIO_IRQ		GPIO4_IRQn
#define CHG_DET_GPIO_RESERVE	RESERVE_GPIO(CHG_DET, CHG_DET_GPIO_PORT, CHG_DET_GPIO_PIN, PID_GPIO)
#define CHG_DET_GPIO_CONFIG 	GPIO_ConfigurePin(CHG_DET_GPIO_PORT, CHG_DET_GPIO_PIN, INPUT_PULLDOWN, PID_GPIO, true)
#define CHG_DET_GPIO_GET		GPIO_GetPinStatus(CHG_DET_GPIO_PORT, CHG_DET_GPIO_PIN)

#if CHG_DET_ACTIVE_LOW
#define CHG_ONLINE \
	(CHG_DET_GPIO_GET == 0)
#else
#define CHG_ONLINE \
	(CHG_DET_GPIO_GET)
#endif

#define CHG_STAT_GPIO_PORT		GPIO_PORT_1
#define CHG_STAT_GPIO_PIN		GPIO_PIN_1
#define CHG_STAT_GPIO_RESERVE	RESERVE_GPIO(CHG_STAT, CHG_STAT_GPIO_PORT, CHG_STAT_GPIO_PIN, PID_GPIO)
#define CHG_STAT_GPIO_CONFIG 	GPIO_ConfigurePin(CHG_STAT_GPIO_PORT, CHG_STAT_GPIO_PIN, INPUT_PULLUP, PID_GPIO, true)
#define CHG_STAT_GPIO_GET		GPIO_GetPinStatus(CHG_STAT_GPIO_PORT, CHG_STAT_GPIO_PIN)

#if CHG_STAT_ACTIVE_LOW
#define BATT_CHARGING \
	(CHG_STAT_GPIO_GET == 0)
#else
#define BATT_CHARGING \
	(CHG_STAT_GPIO_GET)
#endif

#if 0
#define LDO_P3V3_GPIO_PORT		GPIO_PORT_0
#define LDO_P3V3_GPIO_PIN		GPIO_PIN_2
#define LDO_P3V3_GPIO_RESERVE	RESERVE_GPIO(LDO_P3V3, LDO_P3V3_GPIO_PORT, LDO_P3V3_GPIO_PIN, PID_GPIO)
#define LDO_P3V3_GPIO_CONFIG 	GPIO_ConfigurePin(LDO_P3V3_GPIO_PORT, LDO_P3V3_GPIO_PIN, OUTPUT, PID_GPIO, false)
#define LDO_P3V3_OPEN			GPIO_SetActive(LDO_P3V3_GPIO_PORT, LDO_P3V3_GPIO_PIN)
#define LDO_P3V3_CLOSE			GPIO_SetInactive(LDO_P3V3_GPIO_PORT, LDO_P3V3_GPIO_PIN)
#else
#define SENSOR_RST_GPIO_PORT	GPIO_PORT_0
#define SENSOR_RST_GPIO_PIN		GPIO_PIN_2
#define SENSOR_RST_GPIO_RESERVE	RESERVE_GPIO(SENSOR_RST, SENSOR_RST_GPIO_PORT, SENSOR_RST_GPIO_PIN, PID_GPIO)
#define SENSOR_RST_GPIO_CONFIG 	GPIO_ConfigurePin(SENSOR_RST_GPIO_PORT, SENSOR_RST_GPIO_PIN, OUTPUT, PID_GPIO, false)
#define SENSOR_RST_OPEN			GPIO_SetActive(SENSOR_RST_GPIO_PORT, SENSOR_RST_GPIO_PIN)
#define SENSOR_RST_CLOSE		GPIO_SetInactive(SENSOR_RST_GPIO_PORT, SENSOR_RST_GPIO_PIN)
#endif

#define KEY_GPIO_RESERVE(index) \
	RESERVE_GPIO(KEY##index, KEY##index##_GPIO_PORT, KEY##index##_GPIO_PIN, PID_GPIO)

#if KEY_ACTIVE_LOW
#define KEY_GPIO_CONFIG(port, pin) \
	GPIO_ConfigurePin(port, pin, INPUT_PULLUP, PID_GPIO, true)

#define KEY_GET_STATUS(port, pin) \
	(GPIO_GetPinStatus(port, pin) == 0)
#else
#define KEY_GPIO_CONFIG(port, pin) \
	GPIO_ConfigurePin(port, pin, INPUT_PULLDOWN, PID_GPIO, true)

#define KEY_GET_STATUS(port, pin) \
	GPIO_GetPinStatus(port, pin)
#endif

#define KEY3_GPIO_PORT			GPIO_PORT_2
#define KEY3_GPIO_PIN			GPIO_PIN_1

#define KEY2_GPIO_PORT			GPIO_PORT_2
#define KEY2_GPIO_PIN			GPIO_PIN_2

#define KEY4_GPIO_PORT			GPIO_PORT_2
#define KEY4_GPIO_PIN			GPIO_PIN_3

#define KEY1_GPIO_PORT			GPIO_PORT_2
#define KEY1_GPIO_PIN			GPIO_PIN_4

#if 0
#define POSITION_GPIO_PORT		GPIO_PORT_2
#define POSITION1_GPIO_PIN		GPIO_PIN_5
#define POSITION2_GPIO_PIN		GPIO_PIN_7
#define POSITION3_GPIO_PIN		GPIO_PIN_8

#define POSITION_GPIO_READ_PORT \
	GetWord16(GPIO_BASE + (POSITION_GPIO_PORT << 5))

#define POSITION_GPIO_RESERVE(index) \
	RESERVE_GPIO(POSITION##index, POSITION_GPIO_PORT, POSITION##index##_GPIO_PIN, PID_GPIO)

#define POSITION_GPIO_CONFIG(index) \
	GPIO_ConfigurePin(POSITION_GPIO_PORT, POSITION##index##_GPIO_PIN, INPUT_PULLDOWN, PID_GPIO, true)
#endif

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
