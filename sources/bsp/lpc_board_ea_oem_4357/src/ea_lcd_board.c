/*****************************************************************************
 *
 *   Copyright(C) 2011, Embedded Artists AB
 *   All rights reserved.
 *
 ******************************************************************************
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * Embedded Artists AB assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. Embedded Artists AB
 * reserves the right to make changes in the software without
 * notification. Embedded Artists AB also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
 *****************************************************************************/


/******************************************************************************
 * Includes
 *****************************************************************************/

#include "board.h"
#include <stdlib.h>
#include <string.h>

#include "lcd_params.h"
//#include "lcd_driver.h"
#include "ea_lcd_board.h"
#include "lcdb_eeprom.h"



/******************************************************************************
 * Defines and typedefs
 *****************************************************************************/

#define LCDB_I2C_PORT (LPC_I2C0)
#define LCDB_PCA9532_I2C_ADDR    (0x64)

/* PCA9532 registers*/
#define LCDB_PCA9532_INPUT0   0x00
#define LCDB_PCA9532_INPUT1   0x01
#define LCDB_PCA9532_PSC0     0x02
#define LCDB_PCA9532_PWM0     0x03
#define LCDB_PCA9532_PSC1     0x04
#define LCDB_PCA9532_PWM1     0x05
#define LCDB_PCA9532_LS0      0x06
#define LCDB_PCA9532_LS1      0x07
#define LCDB_PCA9532_LS2      0x08
#define LCDB_PCA9532_LS3      0x09
#define LCDB_PCA9532_AUTO_INC 0x10

#define LCDB_LS_MODE_ON     0x01
#define LCDB_LS_MODE_BLINK0 0x02
#define LCDB_LS_MODE_BLINK1 0x03

/* For LCD Board */
#define LCDB_CTRL_3V3     0x0001
#define LCDB_CTRL_5V      0x0002
#define LCDB_CTRL_DISP_EN 0x0010
#define LCDB_CTRL_BL_EN   0x0080
#define LCDB_CTRL_BL_C    0x0100

/* For Display Expansion Board */
#define LCDB_LVDS_PWR_EN  0x0000
#define LCDB_ADV_PSAVE    0x0004
#define LCDB_LVDS_EN      0x0010
#define LCDB_RESET        0x0040
#define LCDB_OSC_SEL_A    0x0100
#define LCDB_OSC_SEL_B    0x0200
#define LCDB_OSC_SEL_C    0x0400

/* Common for both Boards */
#define LCDB_EEPROM_WP    0x8000


#define LCDB_MAGIC 0xEA01CDAE

#define LCDB_NAME_BUF_SZ 30

typedef struct {
  uint32_t magic;        // magic number
  uint8_t  lcd_name[LCDB_NAME_BUF_SZ]; // LCD name
  uint8_t  lcd_mfg[LCDB_NAME_BUF_SZ];  // manufacturer name
  uint16_t lcdParamOff;  // offset to LCD parameters
  uint16_t initOff;      // offset to init sequence string                            
  uint16_t pdOff;        // offset to power down sequence string
  uint16_t tsOff;        // offset to touch parameters
  uint16_t end;          // end offset
} eaLcdbStore_t;

#define LCDB_BUF_SZ 256

/*
 * Set which sequence string version that is supported
 */
#define LCDB_SEQ_VER 1

/*
 * Data used to store the calibration information in the LCD board's storage
 */
#define TOUCH_MAGIC  0xEA0000EA
#define TOUCH_OFF    0x1000
typedef struct
{
	uint32_t magic;
	CALIBDATA_T data;
} eaTouchStore_t;



/******************************************************************************
 * External global variables
 *****************************************************************************/

/******************************************************************************
 * Local variables
 *****************************************************************************/

static uint16_t blink0Shadow = 0;
static uint16_t blink1Shadow = 0;
static uint16_t ledStateShadow = 0;

static char lcdb_buf[LCDB_BUF_SZ];
static LCD_CONFIG_T lcdb_lcdParams;

/******************************************************************************
 * Local Functions
 *****************************************************************************/

static Status I2CWrite(uint32_t addr, uint8_t* buf, uint32_t len) 
{
	I2CM_XFER_T i2cData;

	i2cData.slaveAddr = addr;
	i2cData.options = 0;
	i2cData.status = 0;
	i2cData.txBuff = buf;
	i2cData.txSz = len;
	i2cData.rxBuff = NULL;
	i2cData.rxSz = 0;

	if (Chip_I2CM_XferBlocking(LPC_I2C0, &i2cData) == 0) {
		return ERROR;
	}
	return SUCCESS;
}



/**
 * Convert a string to an integer
 *
 * @params str - the string to convert
 * @params len - length of the string (it might not be null terminated)
 */
static uint32_t str_to_uint(char* str, uint32_t len) 
{
  uint32_t val = 0;

  while(len > 0 && *str <= '9' && *str >= '0') {
    val = (val * 10) + (*str - '0');
    str++;
    len--;
  }

  return val;
}

/**
 * Configure the pins for the LCD controller
 */
static void pinConfig(void)
{
	/* Moved to board_sysinit.c */
}

/**
 * Helper function to set LED states
 */
static void setLsStates(uint16_t states, uint8_t* ls, uint8_t mode)
{
#define IS_LED_SET(bit, x) ( ( ((x) & (bit)) != 0 ) ? 1 : 0 )

    int i = 0;

    for (i = 0; i < 4; i++) {

        ls[i] |= ( (IS_LED_SET(0x0001, states)*mode << 0)
                | (IS_LED_SET(0x0002, states)*mode << 2)
                | (IS_LED_SET(0x0004, states)*mode << 4)
                | (IS_LED_SET(0x0008, states)*mode << 6) );

        states >>= 4;
    }
}

/**
 * Set LEDs (PCA9532 on LCD board)
 */
static void setLeds(void)
{
    uint8_t buf[5];
    uint8_t ls[4] = {0,0,0,0};
    uint16_t states = ledStateShadow;

    /* LEDs in On/Off state */
    setLsStates(states, ls, LCDB_LS_MODE_ON);

    /* set the LEDs that should blink */
    setLsStates(blink0Shadow, ls, LCDB_LS_MODE_BLINK0);
    setLsStates(blink1Shadow, ls, LCDB_LS_MODE_BLINK1);

    buf[0] = LCDB_PCA9532_LS0 | LCDB_PCA9532_AUTO_INC;
    buf[1] = ls[0];
    buf[2] = ls[1];
    buf[3] = ls[2];
    buf[4] = ls[3];
    I2CWrite(LCDB_PCA9532_I2C_ADDR, buf, 5);
}

/**
 * Set LED states (on or off).
 *
 * Params:
 *    [in]  ledOnMask  - The LEDs that should be turned on. This mask has
 *                       priority over ledOffMask
 *    [in]  ledOffMask - The LEDs that should be turned off.
 *
 */
static void lcdb_pca9532_setLeds (uint16_t ledOnMask, uint16_t ledOffMask)
{
    /* turn off leds */
    ledStateShadow &= (~(ledOffMask) & 0xffff);

    /* ledOnMask has priority over ledOffMask */
    ledStateShadow |= ledOnMask;

    /* turn off blinking */
    blink0Shadow &= (~(ledOffMask) & 0xffff);
    blink1Shadow &= (~(ledOffMask) & 0xffff);

    setLeds();
}

/**
 *  Set the blink period for PWM0. Valid values are 0 - 255 where 0
 *  means 152 Hz and 255 means 0.59 Hz. A value of 151 means 1 Hz.
 *
 * Params:
 *    [in]  period  - the period for pwm0
 *
 */
static void lcdb_pca9532_setBlink0Period(uint8_t period)
{
    uint8_t buf[2];

    buf[0] = LCDB_PCA9532_PSC0;
    buf[1] = period;
    I2CWrite(LCDB_PCA9532_I2C_ADDR, buf, 2);
}

/**
 * Set the duty cycle for PWM0. Valid values are 0 - 100. 25 means the LED
 * is on 25% of the period.
 *
 * Params:
 *    [in]  duty  - duty cycle
 *
 */
static void lcdb_pca9532_setBlink0Duty(uint8_t duty)
{
    uint8_t buf[2];
    uint32_t tmp = duty;
    if (tmp > 100) {
        tmp = 100;
    }

    tmp = (255 * tmp)/100;

    buf[0] = LCDB_PCA9532_PWM0;
    buf[1] = tmp;
    I2CWrite(LCDB_PCA9532_I2C_ADDR, buf, 2);
}

/**
 *  Set the LEDs that should blink with rate and duty cycle from PWM0.
 *  Blinking is turned off with pca9532_setLeds.
 *
 * Params:
 *    [in]  ledMask  - LEDs that should blink.
 *
 */
static void lcdb_pca9532_setBlink0Leds(uint16_t ledMask)
{
    blink0Shadow |= ledMask;
    setLeds();
}

/**
 * Enable/disable write protect for the EEPROM on the 
 * LCD Board
 *
 * Params:
 *    [in]  enable  - set to 1 to enable write protect;
 *       otherwise set to 0 to disable write protect.
 *
 */
static void ea_lcdb_ctrl_wp(uint32_t enable)
{
  if (enable) {
    lcdb_pca9532_setLeds(0, LCDB_EEPROM_WP);
  } else {
    lcdb_pca9532_setLeds(LCDB_EEPROM_WP, 0);
  }
}

/******************************************************************************
 *
 * Description:
 *    Enable/disable 3V3 signal
 *
 *****************************************************************************/
static void ea_lcdb_ctrl_3v3(uint32_t enable)
{
  if (enable) {
    lcdb_pca9532_setLeds(LCDB_CTRL_3V3, 0);
  } else {
    lcdb_pca9532_setLeds(0, LCDB_CTRL_3V3);
  }

}

/******************************************************************************
 *
 * Description:
 *    Enable/disable 5V signal
 *
 *****************************************************************************/
static void ea_lcdb_ctrl_5v(uint32_t enable)
{
  if (enable) {
    lcdb_pca9532_setLeds(LCDB_CTRL_5V, 0);
  } else {
    lcdb_pca9532_setLeds(0, LCDB_CTRL_5V);
  }
}

/******************************************************************************
 *
 * Description:
 *    Enable/disable display enable signal
 *
 *****************************************************************************/
static void ea_lcdb_ctrl_display(uint32_t enable)
{
  if (!enable) {
    lcdb_pca9532_setLeds(LCDB_CTRL_DISP_EN, 0);
  } else {
    lcdb_pca9532_setLeds(0, LCDB_CTRL_DISP_EN);
  }
}


/**
 * Check if the sequence string version can be handled.
 *
 */
static lcdb_result_t checkVersion(char* v, uint32_t len)
{
  uint32_t ver = str_to_uint(v, len);

  if (ver > LCDB_SEQ_VER) {
    return LCD_RESULT_SEQ_VER;  
  }

  return LCDB_RESULT_OK;
}

/**
 * Execute a delay request
 */
static lcdb_result_t execDelay(char* del, uint32_t len)
{
	Board_DelayMs(str_to_uint(del, len));

  return LCDB_RESULT_OK;
}


/**
 * Execute a control request (PCA9532)
 */
static lcdb_result_t execSeqCtrl(char* cmd, uint32_t len)
{

  switch (*cmd++) {
    // display enable
    case 'd':
      ea_lcdb_ctrl_display(*cmd == '1');
      break;
    
    // backlight contrast
    case 'c':
      ea_lcdb_ctrl_backlightContrast(str_to_uint(cmd, len));
      break;

    // 3v3 enable
    case '3':
      ea_lcdb_ctrl_3v3(*cmd == '1');
      break;

    // 5v enable    
    case '5':
      ea_lcdb_ctrl_5v(*cmd == '1');
      break;

  }

  return LCDB_RESULT_OK;
}

/**
 * Parse and run the init sequence string
 */
static lcdb_result_t parseInitString(char* str, 
		LCD_CONFIG_T* lcdParams, int32_t* pDev)
{
  char* c = NULL;
  uint32_t len = 0;
  lcdb_result_t result = LCDB_RESULT_OK;

  if (str == NULL) {
    return LCDB_RESULT_INVALID_CMD_STR;
  }

  while(*str != '\0') {

    // skip whitespaces
    while(*str == ' ') {
      str++;
    }
    
    c = str;

    // find end of command
    while(*str != ',' && *str != '\0') {
      str++;
    }

    len = (str-c);

    if (*str == ',') {
      str++;
    }

    switch (*c++) {

      case 'v':
        result = checkVersion(c, len-1);
        break;

      // sequence control command (pca9532)
      case 'c':
        execSeqCtrl(c, len-1);
        break;
      
      // delay
      case 'd':
        execDelay(c, len-1);
        break;

      // open lcd (init LCD controller)
      case 'o':
        
        if (lcdParams != NULL) {
          //*pDev = lcd_open((int32_t)lcdParams);
        	Chip_LCD_Init(LPC_LCD, lcdParams);
        }

        else {
          result = LCDB_RESULT_INVALID_ARG;
        }

        break;
    }

    if (result != LCDB_RESULT_OK) {
      break;
    }
  }

  return result;
}

/**
 * Get the store for LCD parameters
 */
static lcdb_result_t ea_lcdb_getStore(eaLcdbStore_t* store) 
{
  if (lcdb_eeprom_read((uint8_t*)store, 0, sizeof(eaLcdbStore_t)) == -1)
  {
    return LCD_RESULT_STORAGE;
  }
  return LCDB_RESULT_OK;
}

/******************************************************************************
 *
 * Description:
 *    Get stored init sequence
 *
 * Params:
 *    [in] buffer: Stored init string is copied here.
 *    [in] len: length of the buffer
 *
 * Return:
 *   Error code
 *
 *****************************************************************************/
static lcdb_result_t ea_lcdb_getInitSeq(char* buf, uint32_t len)
{
  eaLcdbStore_t h;
  lcdb_result_t res;

  res = ea_lcdb_getStore(&h);
  if (res != LCDB_RESULT_OK) {
    return res;
  }

  if (h.magic != LCDB_MAGIC) {
    return LCD_RESULT_STORAGE;
  }

  if ((h.pdOff-h.initOff) > len) {
    return LCD_RESULT_BUF_SMALL;
  }

  lcdb_eeprom_read((uint8_t*)buf, h.initOff,
    (h.pdOff-h.initOff));

  return LCDB_RESULT_OK;
}

/******************************************************************************
 * Public Functions
 *****************************************************************************/


/******************************************************************************
 *
 * Description:
 *    Open the LCD Board device
 *
 * Params:
 *    [in] lcdParams: The LCD parameters to initialize the LCD controller with.
 *                    If this value is NULL the paramters will be read from 
 *                    storage   
 *
 *    [in] initSeqStr: The initialization sequence string to parse and run
 *                     when initializing the display.    
 *                     If this value is NULL the paramters will be read from 
 *                     storage
 *
 *    [out] pDev:      Device handle
 *
 * Return:
 *   Error code
 *
 *****************************************************************************/
lcdb_result_t ea_lcdb_open(LCD_CONFIG_T* lcdParams, char* initSeqStr, int32_t *pDev)
{
  lcdb_result_t result = LCDB_RESULT_OK;
    
  pinConfig();
  
  if (pDev == NULL) {
    return LCDB_RESULT_INVALID_ARG;
  }

  // load LCD params from eeprom
  if (lcdParams == NULL) {
    result = ea_lcdb_getLcdParams(&lcdb_lcdParams);
    lcdParams = &lcdb_lcdParams;
  }

  // load init sequence from eeprom
  if (result == LCDB_RESULT_OK && initSeqStr == NULL) {
    result = ea_lcdb_getInitSeq(lcdb_buf, LCDB_BUF_SZ);
    initSeqStr = lcdb_buf;
  }

  if (result != LCDB_RESULT_OK) {
    return result;
  } 

  // parse and run init string
  return parseInitString(initSeqStr, lcdParams, pDev);
}

/******************************************************************************
 *
 * Description:
 *    Enable the display. Must be called after open in order for the display
 *    to be completely enabled.
 *
 * Params:
 *    [in] dev: Device handle as returned by open.
 *
 *    [in] framebuf: Framebuffer handle to be used by the LCD controller
 *
 * Return:
 *   Error code
 *
 *****************************************************************************/
lcdb_result_t ea_lcdb_enable(int32_t dev, int32_t framebuf)
{
  Chip_LCD_PowerOn(LPC_LCD);
  Chip_LCD_SetUPFrameBuffer(LPC_LCD,  (void *) framebuf);

  return LCDB_RESULT_OK;
}

/******************************************************************************
 *
 * Description:
 *    Close the LCD board device
 *
 * Params:
 *    [in] dev: Device handle as returned by open.
 *
 * Return:
 *   Error code
 *
 *****************************************************************************/
lcdb_result_t ea_lcdb_close(int32_t devid)
{
  // TODO use the power down string

  Chip_LCD_PowerOff(LPC_LCD);

  return LCDB_RESULT_OK;
}

/******************************************************************************
 *
 * Description:
 *    Get stored LCD parameters
 *
 * Params:
 *    [out] lcdParams: Stored LCD parameters are copied here. 
 *
 * Return:
 *   Error code
 *
 *****************************************************************************/
lcdb_result_t ea_lcdb_getLcdParams(LCD_CONFIG_T* lcdParams)
{
  eaLcdbStore_t h;
  LCD_PARAM_T tmp;

  ea_lcdb_getStore(&h);

  if (h.magic != LCDB_MAGIC) {
    return LCD_RESULT_STORAGE;
  }

  lcdb_eeprom_read((uint8_t*)&tmp, h.lcdParamOff,
    (h.initOff-h.lcdParamOff));  

  // Convert from the stored format to the LPCOpen format
	lcdParams->HBP          = tmp.h_back_porch;          /*!< Horizontal back porch in clocks */
	lcdParams->HFP          = tmp.h_front_porch;         /*!< Horizontal front porch in clocks */
	lcdParams->HSW          = tmp.h_sync_pulse_width;    /*!< HSYNC pulse width in clocks */
	lcdParams->PPL          = tmp.pixels_per_line;       /*!< Pixels per line */
	lcdParams->VBP          = tmp.v_back_porch;          /*!< Vertical back porch in clocks */
	lcdParams->VFP          = tmp.v_front_porch;         /*!< Vertical front porch in clocks */
	lcdParams->VSW          = tmp.v_sync_pulse_width;    /*!< VSYNC pulse width in clocks */
	lcdParams->LPP          = tmp.lines_per_panel;       /*!< Lines per panel */
	lcdParams->IOE          = tmp.invert_output_enable;  /*!< Invert output enable, 1 = invert */
	lcdParams->IPC          = tmp.invert_panel_clock;    /*!< Invert panel clock, 1 = invert */
	lcdParams->IHS          = tmp.invert_hsync;          /*!< Invert HSYNC, 1 = invert */
	lcdParams->IVS          = tmp.invert_vsync;          /*!< Invert VSYNC, 1 = invert */
	lcdParams->ACB          = tmp.ac_bias_frequency;     /*!< AC bias frequency in clocks (not used) */
	lcdParams->Dual         = tmp.dual_panel;	         /*!< Dual panel, 1 = dual panel display */
	lcdParams->color_format = LCD_COLOR_FORMAT_BGR;      /*!< BGR or RGB */
	lcdParams->Clock        = tmp.optimal_clock;         /*!< Wanted panel clock in Hz or 0 to use GP_CLKIN */

	switch (tmp.lcd_panel_type) {
	case TFT:
	case ADTFT:
	case HRTFT:
		lcdParams->LCD = LCD_TFT;
		break;
	case MONO_4BIT:
		lcdParams->LCD = LCD_MONO_4;
		break;
	case MONO_8BIT:
		lcdParams->LCD = LCD_MONO_8;
		break;
	case CSTN:
		lcdParams->LCD = LCD_CSTN;
		break;
	default:
		lcdParams->LCD = LCD_TFT;
		break;
	}

	// In LCD_PARAM_T the value is in bits per pixel, in LCD_CONFIG_T the value
	// is what goes into the LPC_LCD->CTRL register's LCDBPP bits so it must be
	// converted.
	switch(tmp.bits_per_pixel) {  /*!< Maximum bits per pixel the display supports */
	case  1:  lcdParams->BPP = 0; break;
	case  2:  lcdParams->BPP = 1; break;
	case  4:  lcdParams->BPP = 2; break;
	case  8:  lcdParams->BPP = 3; break;
	/*case 16:  lcdParams->BPP = 4; break; Cannot distinguish this from the 5:6:5 mode*/
	case 24:  lcdParams->BPP = 5; break;
	case 12:  lcdParams->BPP = 7; break; /* 4:4:4 */
	case 16:  lcdParams->BPP = 6; break; /* 5:6:5 */
	default:
		lcdParams->BPP = 6; break; /* 5:6:5 */
	}

	// The calculation of vertical back and front porches are different compared to
	// what we store, so it needs to be adjusted
	lcdParams->VFP++;
	lcdParams->VBP++;


  return LCDB_RESULT_OK;  
}

/******************************************************************************
 *
 * Description:
 *    Get display name. This is fetched from eeprom
 *
 * Params:
 *    [in] buf: the display name is copied to this buffer 
 *    [in] len: size of the buffer
 *
 * Return:
 *   Error code
 *
 *****************************************************************************/
lcdb_result_t ea_lcdb_getDisplayName(char* buf, uint32_t len)
{
  eaLcdbStore_t h;

  ea_lcdb_getStore(&h);

  if (h.magic != LCDB_MAGIC) {
    return LCD_RESULT_STORAGE;
  }

  if (len < LCDB_NAME_BUF_SZ) {
    return LCD_RESULT_BUF_SMALL;
  }

  strncpy(buf, (char*)h.lcd_name, LCDB_NAME_BUF_SZ);

  return LCDB_RESULT_OK;
}

/******************************************************************************
 *
 * Description:
 *    Get display manufacturer. This is fetched from eeprom
 *
 * Params:
 *    [in] buf: the display mfg is copied to this buffer 
 *    [in] len: size of the buffer
 *
 * Return:
 *   Error code
 *
 *****************************************************************************/
lcdb_result_t ea_lcdb_getDisplayMfg(char* buf, uint32_t len)
{
  eaLcdbStore_t h;

  ea_lcdb_getStore(&h);

  if (h.magic != LCDB_MAGIC) {
    return LCD_RESULT_STORAGE;
  }

  if (len < LCDB_NAME_BUF_SZ) {
    return LCD_RESULT_BUF_SMALL;
  }

  strncpy(buf, (char*)h.lcd_mfg, LCDB_NAME_BUF_SZ);

  return LCDB_RESULT_OK;
}


/******************************************************************************
 *
 * Description:
 *    Get stored power down sequence
 *
 * Params:
 *    [in] buffer: Stored string is copied here. 
 *    [in] len: length of the buffer
 *
 * Return:
 *   Error code
 *
 *****************************************************************************/
//lcdb_result_t ea_lcdb_getPowerDownSeq(char* buf, uint32_t len)
//{
//  eaLcdbStore_t h;
//
//  ea_lcdb_getStore(&h);
//
//  if (h.magic != LCDB_MAGIC) {
//    return LCD_RESULT_STORAGE;
//  }
//
//  if ((h.tsOff-h.pdOff) > len) {
//    return LCD_RESULT_BUF_SMALL;
//  }
//
//  lcdb_eeprom_read((uint8_t*)buf, h.pdOff,
//    (h.tsOff-h.pdOff));
//
//  return LCDB_RESULT_OK;
//}

/******************************************************************************
 *
 * Description:
 *    Get stored Touch parameters
 *
 * Params:
 *    [out] touchParams: Stored Touch parameters are copied here. 
 *
 * Return:
 *   Error code
 *
 *****************************************************************************/
lcdb_result_t ea_lcdb_getTouchParams(eaLcdbTouchParams_t* touchParams) 
{
  eaLcdbStore_t h;

  ea_lcdb_getStore(&h);

  if (h.magic != LCDB_MAGIC) {
    return LCD_RESULT_STORAGE;
  }

  lcdb_eeprom_read((uint8_t*)touchParams, h.tsOff, 
    (h.end-h.tsOff));  

  
  return LCDB_RESULT_OK;  
}



/******************************************************************************
 *
 * Description:
 *    Enable/disable LVDS power enable
 *
 *****************************************************************************/
//void ea_lcdb_ctrl_lvds_pwr(uint32_t enable)
//{
//  if (enable) {
//    lcdb_pca9532_setLeds(LCDB_LVDS_PWR_EN, 0);
//  } else {
//    lcdb_pca9532_setLeds(0, LCDB_LVDS_PWR_EN);
//  }
//}

/******************************************************************************
 *
 * Description:
 *    Selects which oscillator to use
 *
 * Params:
 *    [in] oscillator: which of the four spread spectrum oscillators to use
 *
 * Return:
 *    Error code
 *
 *****************************************************************************/
lcdb_result_t ea_lcdb_ctrl_selectOscillator(osc_e oscillator)
{
  uint32_t on;
  switch (oscillator)
  {
    case OSC_25_175MHZ:
      on = 0;//LCDB_OSC_SEL_A | LCDB_OSC_SEL_B | LCDB_OSC_SEL_C;
      break;

    case OSC_27MHZ:
      on = LCDB_OSC_SEL_A;//LCDB_OSC_SEL_B | LCDB_OSC_SEL_C;
      break;

    case OSC_36MHZ:
      on = LCDB_OSC_SEL_B;//LCDB_OSC_SEL_A | LCDB_OSC_SEL_C;
      break;

    case OSC_USER_DEFINED:
      on = LCDB_OSC_SEL_A | LCDB_OSC_SEL_B;//LCDB_OSC_SEL_C;
      break;

    default:
      return LCDB_RESULT_INVALID_ARG;
  }

  lcdb_pca9532_setLeds(on, (LCDB_OSC_SEL_A | LCDB_OSC_SEL_B | LCDB_OSC_SEL_C));
  return LCDB_RESULT_OK;
}


/******************************************************************************
 *
 * Description:
 *    Set backlight contrast
 *
 * Params:
 *   [in] value - backlight value; valid values 0-100
 *
 *****************************************************************************/
void ea_lcdb_ctrl_backlightContrast(uint32_t value)
{
  lcdb_pca9532_setBlink0Duty(100-value);
  lcdb_pca9532_setBlink0Period(0);
  lcdb_pca9532_setBlink0Leds(LCDB_CTRL_BL_C);
}

/******************************************************************************
 *
 * Description:
 *    Store new LCD parameters and sequence strings in the LCD board
 *    storage.
 *
 * Params:
 *    [in] lcdName: Name of display
 *    [in] lcdMfg:  The manufacturer of the display
 *    [in] lcdParams:  LCD paramters for the LCD controller
 *    [in] initSeqStr:  The initialization sequence string
 *    [in] pdSeqStr:  The power down sequence string
 *    [in] touch: Touch controller parameters
 *
 * Return:
 *   Error code
 *
 *****************************************************************************/
//lcdb_result_t ea_lcdb_store_params(
//  char* lcdName,
//  char* lcdMfg,
//  LCD_PARAM_T* lcdParams,
//  char* initSeqStr,
//  char* pdSeqStr,
//  eaLcdbTouchParams_t* touch)
//{
//  eaLcdbStore_t h;
//
//  if (lcdName == NULL || lcdMfg == NULL || lcdParams == NULL
//    || initSeqStr == NULL || pdSeqStr == NULL) {
//    return LCDB_RESULT_INVALID_ARG;
//  }
//
//  h.magic = LCDB_MAGIC;
//  strncpy((char*)h.lcd_name, lcdName, 30);
//  strncpy((char*)h.lcd_mfg, lcdMfg, 30);
//
//  h.lcdParamOff = sizeof(eaLcdbStore_t);
//  h.initOff     = h.lcdParamOff + sizeof(LCD_PARAM_T);
//  h.pdOff       = h.initOff + strlen(initSeqStr)+1;
//  h.tsOff       = h.pdOff + strlen(pdSeqStr)+1;
//  h.end         = h.tsOff + sizeof(eaLcdbTouchParams_t);
//
//  ea_lcdb_ctrl_wp(0);
//  lcdb_eeprom_write((uint8_t*)&h,         0,             h.lcdParamOff);
//  lcdb_eeprom_write((uint8_t*)lcdParams,  h.lcdParamOff, (h.initOff-h.lcdParamOff));
//  lcdb_eeprom_write((uint8_t*)initSeqStr, h.initOff,     (h.pdOff-h.initOff));
//  lcdb_eeprom_write((uint8_t*)pdSeqStr,   h.pdOff,       (h.tsOff-h.pdOff));
//  lcdb_eeprom_write((uint8_t*)touch,      h.tsOff,       (h.end-h.tsOff));
//  ea_lcdb_ctrl_wp(1);
//
//  return LCDB_RESULT_OK;
//}

/******************************************************************************
 *
 * Description:
 *    Reads touch calibration data from the LCD board storage.
 *    This is separate from the display information.
 *
 * Params:
 *    [out] touchData: The read data. Only valid if LCDB_RESULT_OK is returned
 *
 * Return:
 *   Error code
 *
 *****************************************************************************/
lcdb_result_t ea_lcdb_getTouchCalibData(CALIBDATA_T* touchData)
{
	eaTouchStore_t store;

	if (touchData == NULL) {
		return LCDB_RESULT_INVALID_ARG;
	}

	if (lcdb_eeprom_read((uint8_t*) &store, TOUCH_OFF, sizeof(eaTouchStore_t)) == -1) {
		return LCD_RESULT_STORAGE;
	}

	if (store.magic != TOUCH_MAGIC) {
		return LCD_RESULT_STORAGE;
	}

	memcpy(touchData, &store.data, sizeof(CALIBDATA_T));
	return LCDB_RESULT_OK;
}

/******************************************************************************
 *
 * Description:
 *    Stores touch calibration data in the LCD board storage.
 *    This is separate from the display information.
 *
 * Params:
 *    [in] touchData: The data to store
 *
 * Return:
 *   Error code
 *
 *****************************************************************************/
lcdb_result_t ea_lcdb_storeTouchCalibData(const CALIBDATA_T* touchData)
{
	eaTouchStore_t store;

	if (touchData == NULL) {
		return LCDB_RESULT_INVALID_ARG;
	}

	store.magic = TOUCH_MAGIC;
	memcpy(&store.data, touchData, sizeof(CALIBDATA_T));

	ea_lcdb_ctrl_wp(0);
	lcdb_eeprom_write((uint8_t*) &store, TOUCH_OFF, sizeof(eaTouchStore_t));
	ea_lcdb_ctrl_wp(1);

	// readback for validation
	memset(&store, 0xff, sizeof(eaTouchStore_t));
	if (ea_lcdb_getTouchCalibData(&store.data) == LCDB_RESULT_OK) {
		if (memcmp(&store.data, touchData, sizeof(CALIBDATA_T)) == 0) {
			return LCDB_RESULT_OK;
		}
	}
	return LCD_RESULT_STORAGE;
}

