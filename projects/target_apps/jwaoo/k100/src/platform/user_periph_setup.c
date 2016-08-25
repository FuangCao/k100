/**
 ****************************************************************************************
 *
 * @file user_periph_setup.c
 *
 * @brief Peripherals setup and initialization.
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

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"             // SW configuration
#include "user_periph_setup.h"       // peripheral configuration
#include "global_io.h"
#include "gpio.h"
#include "uart.h"                    // UART initialization
#include "jwaoo_hw.h"

#if DEVELOPMENT_DEBUG

void GPIO_reservations(void)
{
/*
* Globally reserved GPIOs reservation
*/

/*
* Application specific GPIOs reservation. Used only in Development mode (#if DEVELOPMENT_DEBUG)

i.e.
    RESERVE_GPIO(DESCRIPTIVE_NAME, GPIO_PORT_0, GPIO_PIN_1, PID_GPIO);    //Reserve P_01 as Generic Purpose I/O
*/

#ifdef CFG_PRINTF_UART2
    RESERVE_GPIO(UART2_TX, UART_TX_GPIO_PORT, UART_TX_GPIO_PIN, PID_UART2_TX);
#ifdef UART_RX_GPIO_PORT
    RESERVE_GPIO(UART2_RX, UART_RX_GPIO_PORT, UART_RX_GPIO_PIN, PID_UART2_RX);
#endif
#endif

	RESERVE_GPIO(SPI_CLK, SPI_CLK_GPIO_PORT, SPI_CLK_GPIO_PIN, PID_SPI_CLK);
	RESERVE_GPIO(SPI_DO, SPI_DO_GPIO_PORT, SPI_DO_GPIO_PIN, PID_SPI_DO);
	RESERVE_GPIO(SPI_DI, SPI_DI_GPIO_PORT, SPI_DI_GPIO_PIN, PID_SPI_DI);
	RESERVE_GPIO(SPI_EN, SPI_CS_GPIO_PORT, SPI_CS_GPIO_PIN, PID_SPI_EN);

	RESERVE_GPIO(I2C_SCL, I2C1_GPIO_PORT, I2C1_SCL_GPIO_PIN, PID_I2C_SCL);
	RESERVE_GPIO(I2C_SDA, I2C1_GPIO_PORT, I2C1_SDA_GPIO_PIN, PID_I2C_SDA);

#ifdef MOTO_GPIO_RESERVE
	MOTO_GPIO_RESERVE;
#endif

#ifdef BATT_LED_GPIO_RESERVE
	BATT_LED_GPIO_RESERVE;
#endif

#ifdef BT_LED_GPIO_RESERVE
	BT_LED_GPIO_RESERVE;
#endif

#ifdef BATT_ADC_GPIO_RESERVE
	BATT_ADC_GPIO_RESERVE;
#endif

#ifdef CHG_DET_GPIO_RESERVE
	CHG_DET_GPIO_RESERVE;
#endif

#ifdef KEY1_GPIO_PORT
	KEY_GPIO_RESERVE(1);
#endif

#ifdef KEY2_GPIO_PORT
	KEY_GPIO_RESERVE(2);
#endif

#ifdef KEY3_GPIO_PORT
	KEY_GPIO_RESERVE(3);
#endif

#ifdef KEY4_GPIO_PORT
	KEY_GPIO_RESERVE(4);
#endif
}
#endif //DEVELOPMENT_DEBUG

void set_pad_functions(void)        // set gpio port function mode
{
#ifdef CFG_PRINTF_UART2
    GPIO_ConfigurePin(UART_TX_GPIO_PORT, UART_TX_GPIO_PIN, OUTPUT, PID_UART2_TX, false);
#ifdef UART_RX_GPIO_PORT
    GPIO_ConfigurePin(UART_RX_GPIO_PORT, UART_RX_GPIO_PIN, INPUT, PID_UART2_RX, false);
#endif
#endif

#ifdef MOTO_GPIO_CONFIG
	MOTO_GPIO_CONFIG;
#endif

#ifdef BATT_LED_GPIO_CONFIG
	BATT_LED_GPIO_CONFIG;
#endif

#ifdef BT_LED_GPIO_CONFIG
	BT_LED_GPIO_CONFIG;
#endif

#ifdef BATT_ADC_GPIO_CONFIG
	BATT_ADC_GPIO_CONFIG;
#endif

#ifdef KEY1_GPIO_PORT
	KEY_GPIO_CONFIG(1);
#endif

#ifdef KEY2_GPIO_PORT
	KEY_GPIO_CONFIG(2);
#endif

#ifdef KEY3_GPIO_PORT
	KEY_GPIO_CONFIG(3);
#endif

#ifdef KEY4_GPIO_PORT
	KEY_GPIO_CONFIG(4);
#endif

#ifdef CHG_DET_GPIO_CONFIG
	CHG_DET_GPIO_CONFIG;
#endif

#ifdef LDO_P3V3_GPIO_CONFIG
	LDO_P3V3_GPIO_CONFIG;
#endif
}

void periph_init(void)
{
    // Power up peripherals' power domain
    SetBits16(PMU_CTRL_REG, PERIPH_SLEEP, 0);
    while (!(GetWord16(SYS_STAT_REG) & PER_IS_UP));

    SetBits16(CLK_16M_REG, XTAL16_BIAS_SH_ENABLE, 1);

    //rom patch
    patch_func();

    //Init pads
    set_pad_functions();

    // (Re)Initialize peripherals
    // i.e.
    //  uart_init(UART_BAUDRATE_115K2, 3);

#ifdef CFG_PRINTF_UART2
    SetBits16(CLK_PER_REG, UART2_ENABLE, 1);
    uart2_init(UART_BAUDRATE_115K2, 3);
#endif

	jwaoo_hw_init();


   // Enable the pads
    SetBits16(SYS_CTRL_REG, PAD_LATCH_EN, 1);
}
