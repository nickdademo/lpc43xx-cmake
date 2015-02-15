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

#include "tfp410.h"

/******************************************************************************
 * Defines and typedefs
 *****************************************************************************/

#define I2C_ADDR_TDA410  (0x70>>1)

/******************************************************************************
 * External global variables
 *****************************************************************************/


/******************************************************************************
 * Local variables
 *****************************************************************************/


/******************************************************************************
 * Local Functions
 *****************************************************************************/

static Status
writeToReg(uint8_t *pBuf, uint16_t len, uint8_t *pBuf2, uint16_t len2) 
{
  I2CM_XFER_T i2cData;

  i2cData.slaveAddr = I2C_ADDR_TDA410;
  i2cData.options = 0;
  i2cData.status = 0;
  i2cData.txBuff = pBuf;
  i2cData.txSz = len;
  i2cData.rxBuff = pBuf2;
  i2cData.rxSz = len2;

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
 *    Initialize the TFP410
 *
 * Return:
 *    SUCCESS or ERROR
 *
 *****************************************************************************/
Status tfp410_init(void)
{
  Status result = ERROR;
  
  uint8_t buff1[10];
  uint8_t buff2[10];

  do
  {
    Board_DelayMs(10);

    //Read chip id
    buff1[0] = 0;
    if (writeToReg(buff1, 1, buff2, 5) == ERROR)
      break;
    if (writeToReg(buff1, 1, buff2, 5) == ERROR)
      break;
    if (writeToReg(buff1, 1, buff2, 5) == ERROR)
      break;
  //  printf("\n\nChip vendor = 0x%x, id = 0x%x, rev = 0x%x\n",
  //    ((uint16_t)buff2[1]<<8)|((uint16_t)buff2[0]), ((uint16_t)buff2[3]<<8)|((uint16_t)buff2[2]), buff2[4]);

    //Program chip
    buff1[0] = 0x08;  //start at register address 0x08
    buff1[1] = 0x01 | 0x02 | 0x04 | 0x00 | 0x10 | 0x20 | 0x00;
    buff1[2] = 0x01 | 0x04 | 0x08 | 0x30;
    buff1[3] = 0x80;
    if (writeToReg(buff1, 4, NULL, 0) == ERROR)
      break;

    //Read chip id
    buff1[0] = 0;
    result = writeToReg(buff1, 1, buff2, 5);
    
  } while(FALSE);
  
  return result;
}

