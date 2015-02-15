/*
 * @brief EA OEM 4357 board file
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2013
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#ifndef __BOARD_H_
#define __BOARD_H_

#include "chip.h"
/* board_api.h is included at the bottom of this file after DEBUG setup */

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup BOARD_EA_OEM_4357 LPC4357 EA OEM board support software API functions
 * @ingroup LPCOPEN_43XX_BOARD_EA4357
 * The board support software API functions provide some simple abstracted
 * functions used across multiple LPCOpen board examples. See @ref BOARD_COMMON_API
 * for the functions defined by this board support layer.<br>
 * @{
 */

/** @defgroup BOARD_EA_OEM_4357_OPTIONS BOARD: LPC4357 EA OEM board build options
 * This board has options that configure its operation at build-time.<br>
 * @{
 */

/** Define DEBUG_ENABLE to enable IO via the DEBUGSTR, DEBUGOUT, and
    DEBUGIN macros. If not defined, DEBUG* functions will be optimized
	out of the code at build time.
 */
#define DEBUG_ENABLE

/** Define DEBUG_SEMIHOSTING along with DEBUG_ENABLE to enable IO support
    via semihosting. You may need to use a C library that supports
	semihosting with this option.
 */
//#define DEBUG_SEMIHOSTING

/** Board UART used for debug output and input using the DEBUG* macros. This
    is also the port used for Board_UARTPutChar, Board_UARTGetChar, and
	Board_UARTPutSTR functions. */
#define DEBUG_UART LPC_USART0

/**
 * @}
 */

/* Board name */
#define BOARD_EA_OEM_4357

/* Build for RMII interface */
#define USE_RMII

/* 8 LEDs connected to a PCA9532, 16 LEDs memory mapped */
#define LED_NUMBER_OF 24

#define JOY_UP              0x01
#define JOY_DOWN            0x02
#define JOY_LEFT            0x04
#define JOY_RIGHT           0x08
#define JOY_PRESS           0x10
#define NO_BUTTON_PRESSED   0x00

#define BUTTONS_BUTTON2     0x01
#define BUTTONS_BUTTON3     0x02
#define BUTTONS_BUTTON4     0x04
#define BUTTONS_BUTTON5     0x08


/* UDA1380 address */
#define I2CDEV_UDA1380_ADDR     (0x34 >> 1)
#define UDA1380_I2C_BUS         I2C0

/* PCA9532 address */
#define I2CDEV_PCA9532_ADDR     (0xC0 >> 1)
#define PCA9532_I2C_BUS         I2C0

/* Frame buffer address for lcd */
#define FRAMEBUFFER_ADDR        0x28000000

/* Address for Memory Mapped LEDs */
#define MEMREG_BASE   0x1e000000

/* Global access to the LCD parameters (only valid after Board_LCD_Init()) */
extern LCD_CONFIG_T EA4357_LCD;
#define BOARD_LCD EA4357_LCD

/* Data for TSC2046 (Touch Screen) calibration */
typedef struct
{
	int32_t refX1;
	int32_t refY1;
	int32_t refX2;
	int32_t refY2;
	int32_t refX3;
	int32_t refY3;
	int32_t scrX1;
	int32_t scrY1;
	int32_t scrX2;
	int32_t scrY2;
	int32_t scrX3;
	int32_t scrY3;
} CALIBDATA_T;

/**
 * @brief	Sets up board specific ADC interface
 * @return	Nothing
 */
void Board_ADC_Init(void);

/**
 * @brief	Sets up board specific I2C interface
 * @param	id	: I2C Interface ID (I2C0, I2C1 ... etc)
 * @return	Nothing
 */
void Board_I2C_Init(I2C_ID_T id);

/**
 * @brief	Sets up I2C Fast Plus mode
 * @param	id	: Must always be I2C0
 * @return	Nothing
 * @note	This function must be called before calling
 *          Chip_I2C_SetClockRate() to set clock rates above
 *          normal range 100KHz to 400KHz. Only I2C0 supports
 *          this mode.
 */
STATIC INLINE void Board_I2C_EnableFastPlus(I2C_ID_T id)
{
	Chip_SCU_I2C0PinConfig(I2C0_FAST_MODE_PLUS);
}

/**
 * @brief	Disable I2C Fast Plus mode and enables default mode
 * @param	id	: Must always be I2C0
 * @return	Nothing
 * @sa		Board_I2C_EnableFastPlus()
 */
STATIC INLINE void Board_I2C_DisableFastPlus(I2C_ID_T id)
{
	Chip_SCU_I2C0PinConfig(I2C0_STANDARD_FAST_MODE);
}

/**
 * @brief	Initializes board specific GPIO Interrupt
 * @return	Nothing
 */
void Board_GPIO_Int_Init(void);

/**
 * @brief	Sets up board specific SDMMC interface
 * @return	Nothing
 */
void Board_SDMMC_Init(void);

/**
 * @brief	Sets up board specific SSP interface
 * @param	pSSP	: Pointer to SSP interface to initialize
 * @return	Nothing
 */
void Board_SSP_Init(LPC_SSP_T *pSSP);

/**
 * @brief	Returns the MAC address assigned to this board
 * @param	mcaddr	: Pointer to 6-byte character array to populate with MAC address
 * @return	Nothing
 */
void Board_ENET_GetMacADDR(uint8_t *mcaddr);

/**
 * @brief	Initialize pin muxing for a UART
 * @param	pUART	: Pointer to UART register block for UART pins to init
 * @return	Nothing
 */
void Board_UART_Init(LPC_USART_T *pUART);

/**
 * @brief	Initialize the LCD interface
 * @return	Nothing
 */
void Board_LCD_Init(void);

/**
 * @brief	Initializes the LCD Controller
 * @return	Nothing
 */
//void Board_InitLCDController(void);

/**
 * @brief	Initialize touchscreen controller
 * @return	Nothing
 */
void Board_InitTouchController(void);

/**
 * @brief	Get touch screen position (Polled mode)
 * @param	pX	: pointer to X position
 * @param	pY	: pointer to Y position
 * @return	true if touch is detected or false if otherwise
 * @note	When using OS use Board_I2C_GetTouchPos(), instead
 *          of this function as this function uses busy wait
 *          to do I2C transfers.
 */
bool Board_GetTouchPos(int16_t *pX, int16_t *pY);

/**
 * @brief	Recalibrate the touch screen with the specified parameters
 * @param	setup	: The data to use, or NULL to clear the current calibration
 * @return	Nothing
 * @note	This function will NOT save the calibration data in non-volatile
 *          memory.
 */
void Board_CalibrateTouch(CALIBDATA_T* setup);

/**
 * @brief	Retrieve the actively used calibration data.
 * @param	setup	: Previously gathered data or NULL to collect new data
 * @return	true if data was retrieved or false if otherwise
 * @note	Use this function to read the calibration data stored in non volatile
 *          memory (if any).
 */
bool Board_GetStoredCalibrationData(CALIBDATA_T* setup);

/**
 * @brief	Store the calibration data in E2PROM on the display module
 * @param	setup	: The data to store
 * @return	true if data was stored or false if otherwise
 * @note	Use this function to store the calibration data in non volatile memory
 *          to avoid having to recalibrate on each startup.
 */
bool Board_StoreCalibrationData(CALIBDATA_T* setup);

/**
 * @brief	Initializes board specific buttons
 * @return	Nothing
 */
void Board_Buttons_Init (void);

/**
 * @brief	Initializes board specific joystick
 * @return	Nothing
 */
void Board_Joystick_Init (void);

/**
 * @brief	Initialize joystick interface on board
 * @return	joystick status: up, down, left or right
 */
uint8_t Joystick_GetStatus (void);

/**
 * @brief	Returns button(s) state on board
 * @return	Returns BUTTONS_BUTTON1 if button1 is pressed
 */
uint32_t Buttons_GetStatus(void);

/**
 * @brief	Sets up board specific I2S interface and UDA1380
 * @param	pI2S	: Pointer to I2S interface to initialize
 * @param	micIn	: If 1 selects MIC as input device, if 0 selects LINE_IN
 * @return	Nothing
 */
void Board_Audio_Init(LPC_I2S_T *pI2S, int micIn);

/**
 * @brief	Initialize DAC interface for the board
 * @param	pDAC	: Pointer to DAC interface to initialize
 * @return	Nothing
 */
void Board_DAC_Init(LPC_DAC_T *pDAC);

/**
 * @}
 */

#include "board_api.h"
#include "lpc_phy.h"
#include "uda1380.h"

#ifdef __cplusplus
}
#endif

#endif /* __BOARD_H_ */
