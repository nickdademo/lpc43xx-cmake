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

/*
 * NOTE: I2C must have been initialized before calling any functions in this
 * file.
 */

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "board.h"
#include "string.h"
#include "stdio.h"
#include "lcdb_eeprom.h"

/******************************************************************************
 * Defines and typedefs
 *****************************************************************************/

#define I2C_PORT (LPC_I2C0)

#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif

#define LCDB_EEPROM_I2C_ADDR  (0x56)

#define LCDB_EEPROM_PAGE_SIZE     32
#define LCDB_EEPROM_TOTAL_SIZE  8192


/******************************************************************************
 * External global variables
 *****************************************************************************/

/******************************************************************************
 * Local variables
 *****************************************************************************/


/******************************************************************************
 * Local Functions
 *****************************************************************************/

static void eepromDelay(void)
{
  volatile int i = 0;
  for (i = 0; i <0x20000; i++);
}

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

static Status I2CRead(uint32_t addr, uint8_t* buf, uint32_t len) 
{
	I2CM_XFER_T i2cData;

	i2cData.slaveAddr = addr;
	i2cData.options = 0;
	i2cData.status = 0;
	i2cData.txBuff = NULL;
	i2cData.txSz = 0;
	i2cData.rxBuff = buf;
	i2cData.rxSz = len;

	if (Chip_I2CM_XferBlocking(LPC_I2C0, &i2cData) == 0) {
		return ERROR;
	}
	return SUCCESS;
}


/******************************************************************************
 * Public Functions
 *****************************************************************************/

/******************************************************************************
 *
 * Description:
 *    Initialize the EEPROM Driver
 *
 *****************************************************************************/
void lcdb_eeprom_init (void)
{
    /* nothing to initialize */
}

/******************************************************************************
 *
 * Description:
 *    Read from the EEPROM
 *
 * Params:
 *   [in] buf - read buffer
 *   [in] offset - offset to start to read from
 *   [in] len - number of bytes to read
 *
 * Returns:
 *   number of read bytes or -1 in case of an error
 *
 *****************************************************************************/
int16_t lcdb_eeprom_read(uint8_t* buf, uint16_t offset, uint16_t len)
{
  int i = 0;
  uint8_t off[2];

  if (len > LCDB_EEPROM_TOTAL_SIZE || offset+len > LCDB_EEPROM_TOTAL_SIZE) {
    return -1;
  }

  off[0] = ((offset >> 8) & 0xff);
  off[1] = (offset & 0xff);

  // 1ms delay needed to prevent misreads from EEPROM
  Board_DelayUs(1000);

  if (I2CWrite((LCDB_EEPROM_I2C_ADDR), off, 2) == SUCCESS)
  {
    for ( i = 0; i < 0x2000; i++);
    if (I2CRead((LCDB_EEPROM_I2C_ADDR), buf, len) == SUCCESS) {
      return len;
    }
  }

  return -1;
}

/******************************************************************************
 *
 * Description:
 *    Write to the EEPROM
 *
 * Params:
 *   [in] buf - data to write
 *   [in] offset - offset to start to write to
 *   [in] len - number of bytes to write
 *
 * Returns:
 *   number of written bytes or -1 in case of an error
 *
 *****************************************************************************/
int16_t lcdb_eeprom_write(uint8_t* buf, uint16_t offset, uint16_t len)
{
  int16_t written = 0;
  uint16_t wLen = 0;
  uint16_t off = offset;
  uint8_t tmp[LCDB_EEPROM_PAGE_SIZE+2];

  if (len > LCDB_EEPROM_TOTAL_SIZE || offset+len > LCDB_EEPROM_TOTAL_SIZE) {
    return -1;
  }

  wLen = LCDB_EEPROM_PAGE_SIZE - (off % LCDB_EEPROM_PAGE_SIZE);
  wLen = MIN(wLen, len);

  while (len) {
    tmp[0] = ((off >> 8) & 0xff);
    tmp[1] = (off & 0xff);
    memcpy(&tmp[2], (void*)&buf[written], wLen);
    I2CWrite((LCDB_EEPROM_I2C_ADDR), tmp, wLen+2);

    /* delay to wait for a write cycle */
    eepromDelay();

    len     -= wLen;
    written += wLen;
    off     += wLen;

    wLen = MIN(LCDB_EEPROM_PAGE_SIZE, len);
  }

  return written;
}
