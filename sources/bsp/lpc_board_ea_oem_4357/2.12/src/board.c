/*
 * @brief EA OEM 4357 board file
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2012
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

#include "board.h"
#include "string.h"
#include "stopwatch.h"

#include "retarget.h"
#include "pca9532.h"
#include "ea_lcd_board.h"
#include "tsc2046_touch.h"

/** @ingroup BOARD_EA_OEM_4357
 * @{
 */

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

#define JOYSTICK_UP_GPIO_PORT_NUM         4
#define JOYSTICK_UP_GPIO_BIT_NUM         10
#define JOYSTICK_DOWN_GPIO_PORT_NUM       4
#define JOYSTICK_DOWN_GPIO_BIT_NUM       13
#define JOYSTICK_LEFT_GPIO_PORT_NUM       4
#define JOYSTICK_LEFT_GPIO_BIT_NUM        9
#define JOYSTICK_RIGHT_GPIO_PORT_NUM      4
#define JOYSTICK_RIGHT_GPIO_BIT_NUM      12
#define JOYSTICK_PRESS_GPIO_PORT_NUM      4
#define JOYSTICK_PRESS_GPIO_BIT_NUM       8


static uint16_t memreg_shadow = 0;

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/* System configuration variables used by chip driver */
const uint32_t ExtRateIn = 0;
const uint32_t OscRateIn = 12000000;

LCD_CONFIG_T EA4357_LCD;

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/* Initializes default settings for UDA1380 */
static Status Board_Audio_CodecInit(int micIn)
{
	/* Reset UDA1380 */
	pca9532_setLeds(1<<6, 0);
	Board_DelayUs(1);
	pca9532_setLeds(0, 1<<6);
	Board_DelayUs(1);

	while (!UDA1380_Init(UDA1380_MIC_IN_LR & - (micIn != 0))) {}

	return SUCCESS;
}

#ifdef CORE_M4
static Status readUniqueID(uint8_t* buf)
{
	int16_t len = 6;
	int i = 0;
	uint8_t off[1]; // 8 bit addressing

	if (buf == NULL) {
		return ERROR;
	}

	off[0] = 0xfa;

	if (Chip_I2C_MasterSend(I2C0, 0x50, off, 1) == 1) {
		for (i = 0; i < 0x2000; i++)
			;
		if (Chip_I2C_MasterRead(I2C0, 0x50, buf, len) == len) {
			if ((buf[0] != 0x00) || (buf[1] != 0x04) || (buf[2] != 0xA3)) {
				DEBUGOUT("EEPROM EUI-48: Invalid manufacturer id\r\n");
				return ERROR;
			}
			return SUCCESS;
		}
	}

	return ERROR;
}
#endif

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/* Basic implementation of delay function. For the M0 core a rough time
   estimate with a for-loop is used instead of the StopWatch functionality
   on the M4. */
void Board_DelayMs(uint32_t ms)
{
#ifdef CORE_M4
	static int initialized = 0;
	if (!initialized) {
		initialized = 1;
		StopWatch_Init();
	}
	StopWatch_DelayMs(ms);
#else
	uint32_t i;
	while(ms >= 70000) {
		Board_DelayMs(10000);
		ms -= 10000;
	}
	i = 30000*ms;
	while (i--);
#endif
}

/* Basic implementation of delay function. For the M0 core a rough time
   estimate with a for-loop is used instead of the StopWatch functionality
   on the M4. */
void Board_DelayUs(uint32_t us)
{
#ifdef CORE_M4
	static int initialized = 0;
	if (!initialized) {
		initialized = 1;
		StopWatch_Init();
	}
	StopWatch_DelayUs(us);
#else
	uint32_t i = 41*us;
	while (i--);
#endif
}

/* Initialize UART pins */
void Board_UART_Init(LPC_USART_T *pUART)
{
	if (pUART == LPC_USART0) {
		Chip_SCU_PinMuxSet(0xF, 10, (SCU_MODE_PULLDOWN | SCU_MODE_FUNC1));		/* PF.10 : UART0_TXD */
		Chip_SCU_PinMuxSet(0xF, 11, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC1));	/* PF.11 : UART0_RXD */
	}
//	else if (pUART == LPC_USART3) {
//		Chip_SCU_PinMuxSet(0x2, 3, (SCU_MODE_PULLDOWN | SCU_MODE_FUNC2));			/* P2.3 : UART3_TXD */
//		Chip_SCU_PinMuxSet(0x2, 4, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC2));	/* P2.4 : UART3_RXD */
//	}
}

/* Initialize debug output via UART for board */
void Board_Debug_Init(void)
{
#if defined(DEBUG_UART)
	Board_UART_Init(DEBUG_UART);

	Chip_UART_Init(DEBUG_UART);
	Chip_UART_SetBaud(DEBUG_UART, 115200);
	Chip_UART_ConfigData(DEBUG_UART, UART_LCR_WLEN8 | UART_LCR_SBS_1BIT | UART_LCR_PARITY_DIS);

	/* Enable UART Transmit */
	Chip_UART_TXEnable(DEBUG_UART);
#endif
}

/* Sends a character on the UART */
void Board_UARTPutChar(char ch)
{
#if defined(DEBUG_UART)
	/* Wait for space in FIFO */
	while ((Chip_UART_ReadLineStatus(DEBUG_UART) & UART_LSR_THRE) == 0) {}
	Chip_UART_SendByte(DEBUG_UART, (uint8_t) ch);
#endif
}

/* Gets a character from the UART, returns EOF if no character is ready */
int Board_UARTGetChar(void)
{
#if defined(DEBUG_UART)
	if (Chip_UART_ReadLineStatus(DEBUG_UART) & UART_LSR_RDR) {
		return (int) Chip_UART_ReadByte(DEBUG_UART);
	}
#endif
	return EOF;
}

/* Outputs a string on the debug UART */
void Board_UARTPutSTR(char *str)
{
#if defined(DEBUG_UART)
	while (*str != '\0') {
		Board_UARTPutChar(*str++);
	}
#endif
}

/* Initializes board LED(s) */
static void Board_LED_Init()
{
	memreg_shadow = 0;
	*((uint16_t*)MEMREG_BASE) = memreg_shadow;
}

/* Sets the state of a board LED to on or off */
void Board_LED_Set(uint8_t LEDNumber, bool On)
{
	if (LEDNumber < 8) {
		if (On) {
			pca9532_setLeds(1<<(LEDNumber+8), 0);
		} else {
			pca9532_setLeds(0, 1<<(LEDNumber+8));
		}
	} else if (LEDNumber < 24) {
		LEDNumber -= 8;
		if (On) {
			memreg_shadow |= (1<<LEDNumber);
		} else {
			memreg_shadow &= ~(1<<LEDNumber);
		}
		*((uint16_t*)MEMREG_BASE) = memreg_shadow;
	}
}

/* Returns the current state of a board LED */
bool Board_LED_Test(uint8_t LEDNumber)
{
	if (LEDNumber < 8) {
		uint32_t val = pca9532_getLedState(0);
		return (bool) ((val >> (LEDNumber+8)) & 0x1);
	} else if (LEDNumber < 24) {
		LEDNumber -= 8;
		return (bool) ((memreg_shadow >> LEDNumber) & 1);
	}
	return false;
}

void Board_LED_Toggle(uint8_t LEDNumber)
{
	if (LEDNumber < 8) {
		uint32_t val = pca9532_getLedState(1); /* use shadow to avoid extra I2C calls */
		bool current = (bool) ((val >> (LEDNumber+8)) & 0x1);
		Board_LED_Set(LEDNumber, !current);
	} else if (LEDNumber < 24) {
		Board_LED_Set(LEDNumber, !Board_LED_Test(LEDNumber));
	}
}

/* Returns the MAC address assigned to this board */
void Board_ENET_GetMacADDR(uint8_t *mcaddr)
{
	const uint8_t boardmac[] = {0x00, 0x60, 0x37, 0x12, 0x34, 0x56};

#ifdef CORE_M4
	if (readUniqueID(mcaddr) != SUCCESS) {
		// Failed to read unique ID so go with the default
		memcpy(mcaddr, boardmac, 6);
	}
#else
	// The readUniqueID() function hangs on the M0 so use hardcoded MAC instead
	memcpy(mcaddr, boardmac, 6);
#endif
}

/* Set up and initialize all required blocks and functions related to the
   board hardware */
void Board_Init(void)
{
	/* Sets up DEBUG UART */
	DEBUGINIT();

	/* Initializes GPIO */
	Chip_GPIO_Init(LPC_GPIO_PORT);

	/* Setup PCA9532 which is used by buttons and LEDs */
	Board_I2C_Init(I2C0);
	pca9532_init();

	/* Initialize LEDs */
	Board_LED_Init();

	/* Initialize Buttons */
	Board_Buttons_Init();

#if defined(USE_RMII)
	Chip_ENET_RMIIEnable(LPC_ETHERNET);
#else
	Chip_ENET_MIIEnable(LPC_ETHERNET);
#endif
}

/* Sets up board specific ADC interface */
void Board_ADC_Init(void)
{
}

/* Sets up board specific I2C interface */
void Board_I2C_Init(I2C_ID_T id)
{
	if (id == I2C1) {
//		/* Configure pin function for I2C1 on PE.13 (I2C1_SDA) and PE.15 (I2C1_SCL) */
//		Chip_SCU_PinMuxSet(0xE, 13, (SCU_MODE_ZIF_DIS | SCU_MODE_INBUFF_EN | SCU_MODE_FUNC2));
//		Chip_SCU_PinMuxSet(0xE, 15, (SCU_MODE_ZIF_DIS | SCU_MODE_INBUFF_EN | SCU_MODE_FUNC2));
	}
	else {
		Chip_SCU_I2C0PinConfig(I2C0_STANDARD_FAST_MODE);

		/* Init I2C */
		Chip_I2C_Init(I2C0);
		Chip_I2C_SetClockRate(I2C0, 100000);
		Chip_I2C_SetMasterEventHandler(I2C0, Chip_I2C_EventHandlerPolling);
	}
}

/* Sets up board specific I2S interface and UDA1380 */
void Board_Audio_Init(LPC_I2S_T *pI2S, int micIn)
{
	I2S_AUDIO_FORMAT_T I2S_Config;

	I2S_Config.SampleRate = 48000;
	I2S_Config.ChannelNumber = 2;	/* 1 is mono, 2 is stereo */
	I2S_Config.WordWidth =  16;		/* 8, 16 or 32 bits */
	Chip_I2S_Init(pI2S);
	Chip_I2S_TxConfig(pI2S, &I2S_Config);

	/* Init UDA1380 CODEC */
	Board_Audio_CodecInit(micIn);
}

/* Initialize the LCD interface */
void Board_LCD_Init(void)
{
	int32_t dev_lcd = 0;
	lcdb_result_t result;

	/* Reset LCD and wait for reset to complete */
	Chip_RGU_TriggerReset(RGU_LCD_RST);
	while (Chip_RGU_InReset(RGU_LCD_RST)) {
	}

	if ((result = ea_lcdb_open(NULL, NULL, &dev_lcd)) == LCDB_RESULT_OK) {

		if ((result = ea_lcdb_getLcdParams(&EA4357_LCD)) != LCDB_RESULT_OK) {
			DEBUGOUT("ea_lcdb_getLcdParams FAILED (%d)\r\n", result);
		}
	}
}

void Board_InitTouchController(void)
{
	CALIBDATA_T c;

    touch_init();

    // Try to calibrate using stored parameters
    if (Board_GetStoredCalibrationData(&c)) {
        Board_CalibrateTouch(&c);
    }
}

/* Poll for Touch position */
bool Board_GetTouchPos(int16_t *pX, int16_t *pY)
{
	int32_t x, y, z;
	touch_xyz(&x, &y, &z);
	*pX = (int16_t)x;
	*pY = (int16_t)y;
	return (z != 0);
}

void Board_CalibrateTouch(CALIBDATA_T* setup)
{
	// Allow the board to be recalibrated (i.e. let all read values
	// be uncalibrated)
	touch_reinit();

	if (setup != NULL) {
		tTouchPoint r1,r2,r3,s1,s2,s3;
		r1.x = setup->refX1;
		r1.y = setup->refY1;
		r2.x = setup->refX2;
		r2.y = setup->refY2;
		r3.x = setup->refX3;
		r3.y = setup->refY3;
		s1.x = setup->scrX1;
		s1.y = setup->scrY1;
		s2.x = setup->scrX2;
		s2.y = setup->scrY2;
		s3.x = setup->scrX3;
		s3.y = setup->scrY3;
		touch_calibrate(r1, r2, r3, s1, s2, s3);
	}
}

bool Board_GetStoredCalibrationData(CALIBDATA_T* setup)
{
	return (ea_lcdb_getTouchCalibData(setup) == LCDB_RESULT_OK);
}

bool Board_StoreCalibrationData(CALIBDATA_T* setup)
{
	return (ea_lcdb_storeTouchCalibData(setup) == LCDB_RESULT_OK);
}

/* Turn on LCD backlight */
void Board_SetLCDBacklight(uint8_t Intensity)
{
	// Workaround for stupid interface. See description in board_api.h
	if (Intensity == 1) {
		Intensity = 100;
	}

	ea_lcdb_ctrl_backlightContrast(Intensity);
}

/* Initializes SDMMC interface */
void Board_SDMMC_Init(void)
{
	Chip_SCU_PinMuxSet(0xC, 4, (SCU_PINIO_FAST | SCU_MODE_FUNC7));	/* PC.4 connected to SDIO_D0 */
	Chip_SCU_PinMuxSet(0xC, 5, (SCU_PINIO_FAST | SCU_MODE_FUNC7));	/* PC.5 connected to SDIO_D1 */
	Chip_SCU_PinMuxSet(0xC, 6, (SCU_PINIO_FAST | SCU_MODE_FUNC7));	/* PC.6 connected to SDIO_D2 */
	Chip_SCU_PinMuxSet(0xC, 7, (SCU_PINIO_FAST | SCU_MODE_FUNC7));	/* PC.7 connected to SDIO_D3 */

	Chip_SCU_PinMuxSet(0xC, 8, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_FUNC7));	/* PC.4 connected to SDIO_CD */
	Chip_SCU_PinMuxSet(0xC, 10, (SCU_PINIO_FAST | SCU_MODE_FUNC7));	/* PC.10 connected to SDIO_CMD */
	Chip_SCU_PinMuxSet(0xC, 0, (SCU_MODE_INACT | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC7));/* PC.0 connected to SDIO_CLK */
}

/* Initializes SSP interface */
void Board_SSP_Init(LPC_SSP_T *pSSP)
{
	if (pSSP == LPC_SSP0) {
		/* Set up clock and power for SSP0 module */
		/* Pins PF.0, PF.1, PF.2, PF.3 already setup as SSP0 SCK, SSEL, MISO and MOSI in board_sysinit.c */
	}
	else {
		return;
	}
}

/* Initializes board specific buttons */
void Board_Buttons_Init(void)
{
    // Nothing to do here. 
    // - SW1 is the reset button and cannot be controlled
    // - Buttons SW2-SW5 are controlled va PCA9532 which is initialized somewhere else.
    // - SW6 is affecting the address pins of the external memory bus and should not be used
}

/* Sets up default states for joystick */
void Board_Joystick_Init(void)
{
	Chip_SCU_PinMuxSet(0xA,  1, (SCU_MODE_PULLUP | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC0));  /* PA_0 as GPIO4[8] */
	Chip_SCU_PinMuxSet(0xA,  2, (SCU_MODE_PULLUP | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC0));  /* PA_1 as GPIO4[9] */
	Chip_SCU_PinMuxSet(0xA,  3, (SCU_MODE_PULLUP | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC0));  /* PA_2 as GPIO4[10] */
	Chip_SCU_PinMuxSet(0x9,  0, (SCU_MODE_PULLUP | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC0));  /* P9_0 as GPIO4[12] */
	Chip_SCU_PinMuxSet(0x9,  1, (SCU_MODE_PULLUP | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC0));  /* P9_1 as GPIO4[13] */

	Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, JOYSTICK_UP_GPIO_PORT_NUM, JOYSTICK_UP_GPIO_BIT_NUM);		/* input */
	Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, JOYSTICK_DOWN_GPIO_PORT_NUM, JOYSTICK_DOWN_GPIO_BIT_NUM);	/* input */
	Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, JOYSTICK_LEFT_GPIO_PORT_NUM, JOYSTICK_LEFT_GPIO_BIT_NUM);	/* input */
	Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, JOYSTICK_RIGHT_GPIO_PORT_NUM, JOYSTICK_RIGHT_GPIO_BIT_NUM);	/* input */
	Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, JOYSTICK_PRESS_GPIO_PORT_NUM, JOYSTICK_PRESS_GPIO_BIT_NUM);	/* input */
}

/* Gets joystick status */
uint8_t Joystick_GetStatus(void)
{

	uint8_t ret = NO_BUTTON_PRESSED;

	if (Chip_GPIO_GetPinState(LPC_GPIO_PORT, JOYSTICK_UP_GPIO_PORT_NUM, JOYSTICK_UP_GPIO_BIT_NUM) == 0) {
		ret |= JOY_UP;
	}
	else if (Chip_GPIO_GetPinState(LPC_GPIO_PORT, JOYSTICK_DOWN_GPIO_PORT_NUM, JOYSTICK_DOWN_GPIO_BIT_NUM) == 0) {
		ret |= JOY_DOWN;
	}
	else if (Chip_GPIO_GetPinState(LPC_GPIO_PORT, JOYSTICK_LEFT_GPIO_PORT_NUM, JOYSTICK_LEFT_GPIO_BIT_NUM) == 0) {
		ret |= JOY_LEFT;
	}
	else if (Chip_GPIO_GetPinState(LPC_GPIO_PORT, JOYSTICK_RIGHT_GPIO_PORT_NUM, JOYSTICK_RIGHT_GPIO_BIT_NUM) == 0) {
		ret |= JOY_RIGHT;
	}
	else if (Chip_GPIO_GetPinState(LPC_GPIO_PORT, JOYSTICK_PRESS_GPIO_PORT_NUM, JOYSTICK_PRESS_GPIO_BIT_NUM) == 0) {
		ret |= JOY_PRESS;
	}

	return ret;
}

/* Gets buttons status */
uint32_t Buttons_GetStatus(void)
{
	uint8_t ret = NO_BUTTON_PRESSED;

	uint32_t val = pca9532_getLedState(0);
	if (val & KEY1) {
		ret |= BUTTONS_BUTTON2;
	}
	if (val & KEY2) {
		ret |= BUTTONS_BUTTON3;
	}
	if (val & KEY3) {
		ret |= BUTTONS_BUTTON4;
	}
	if (val & KEY4) {
		ret |= BUTTONS_BUTTON5;
	}
	return ret;
}

/* Initialize DAC interface for the board */
void Board_DAC_Init(LPC_DAC_T *pDAC)
{
	Chip_SCU_DAC_Analog_Config();
}


/**
 * @}
 */
