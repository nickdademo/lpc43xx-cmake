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

#include "tda19988.h"

/******************************************************************************
 * Defines and typedefs
 *****************************************************************************/

#define I2C_ADDR_HDMI  (0xE0>>1)
#define I2C_ADDR_CEC   (0x68>>1)


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
writeToReg(uint8_t addr, uint8_t *pBuf, uint16_t len, uint8_t *pBuf2, uint16_t len2)
{
  I2CM_XFER_T i2cData;

  i2cData.slaveAddr = addr;
  i2cData.options = 0;
  i2cData.status = 0;
  i2cData.txBuff = pBuf;
  i2cData.txSz = len;
  i2cData.rxBuff = pBuf2;
  i2cData.rxSz = len2;
  if (Chip_I2CM_XferBlocking(LPC_I2C0, &i2cData) == 0) {
    return ERROR;
  }
  Board_DelayUs(10);
  return SUCCESS;
}


static Status setup(tda19988_res res)
{
  Status result = ERROR;
  
  uint8_t buff1[10];
  
  if (res == RES_OTHER)
  {
    // Disable output
    //write register FF of CEC device to 0x00.
    //(68ff = 00h) -> disable everything
    buff1[0] = 0xff;
    buff1[1] = 0x00;
    return writeToReg(I2C_ADDR_CEC, buff1, 2, NULL, 0);
  }
  
  do
  {  
    Board_DelayMs(10);

    //write register FF of CEC device to 0x02.
    //(68ff = 02h) -> Only activate HDMI (no CEC and no RXSense)
    buff1[0] = 0xff;
    buff1[1] = 0x02;
    if (writeToReg(I2C_ADDR_CEC, buff1, 2, NULL, 0) == ERROR)
      break;
    
    //select page 00 of HDMI device :  write 00 in FF
    //(e0ff =00h)
    buff1[0] = 0xff;
    buff1[1] = 0x00;
    if (writeToReg(I2C_ADDR_HDMI, buff1, 2, NULL, 0) == ERROR)
      break;

    //write register A0 of HDMI device to select predefined format :
    //(e0a0 = 00h)       -> select a predefined 640x480p @60Hz format
    buff1[0] = 0xa0;
    buff1[1] = res;
    if (writeToReg(I2C_ADDR_HDMI, buff1, 2, NULL, 0) == ERROR)
      break;

    //write registers A0-A4 of HDMI device to set REFPIX/RELINE
    //(e0a1 = 00h)       -> Reference Pixel preset MSB
    //(e0a2 = 00h)       -> Reference Pixel preset LSB
    //(e0a3 = 00h)       -> Reference Line preset MSB
    //(e0a4 = 00h)       -> Reference Line preset LSB
    buff1[0] = 0xa1;
    buff1[1] = 0x00;
    if (writeToReg(I2C_ADDR_HDMI, buff1, 2, NULL, 0) == ERROR)
      break;
    buff1[0] = 0xa2;
    buff1[1] = 0x14;
    if (writeToReg(I2C_ADDR_HDMI, buff1, 2, NULL, 0) == ERROR)
      break;
    buff1[0] = 0xa3;
    buff1[1] = 0x00;
    if (writeToReg(I2C_ADDR_HDMI, buff1, 2, NULL, 0) == ERROR)
      break;
    buff1[0] = 0xa4;
    buff1[1] = 0x07;
    if (writeToReg(I2C_ADDR_HDMI, buff1, 2, NULL, 0) == ERROR)
      break;
    
    //write register TBG_CTRL1 of HDMI device to select resync method
    //(e0cb = 00h)        -> select
    buff1[0] = 0xcb;
    buff1[1] = 0x00;//0x7c;
    if (writeToReg(I2C_ADDR_HDMI, buff1, 2, NULL, 0) == ERROR)
      break;

    //write registers VIP_CTRL to configure RGB input ports :
    //(e020 = 01h)       -> select VP port configuration
    //(e021 = 23h)
    //(e022 = 47h)
    //(e023 = 16h)
    buff1[0] = 0x20;
    buff1[1] = 0x01;
    if (writeToReg(I2C_ADDR_HDMI, buff1, 2, NULL, 0) == ERROR)
      break;
    buff1[0] = 0x21;
    buff1[1] = 0x23;
    if (writeToReg(I2C_ADDR_HDMI, buff1, 2, NULL, 0) == ERROR)
      break;
    buff1[0] = 0x22;
    buff1[1] = 0x47;
    if (writeToReg(I2C_ADDR_HDMI, buff1, 2, NULL, 0) == ERROR)
      break;
    buff1[0] = 0x23;
    buff1[1] = 0x16;
    if (writeToReg(I2C_ADDR_HDMI, buff1, 2, NULL, 0) == ERROR)
      break;    
    
    //clear register E4 of HDMI device to prevent any test patterns
    //(e0e4 = 00h)
    buff1[0] = 0xe4;
    buff1[1] = 0x00;
    if (writeToReg(I2C_ADDR_HDMI, buff1, 2, NULL, 0) == ERROR)
      break;
    
    //write register F0 of HDMI device to select unrepeated HDMI output :
    //(e0f0 = 00h)
    buff1[0] = 0xf0;
    buff1[1] = 0x00;
    if (writeToReg(I2C_ADDR_HDMI, buff1, 2, NULL, 0) == ERROR)
      break;
    
    result = SUCCESS;
    
  } while (FALSE);
  
  return result;
}

static Status sendToCEC(uint8_t reg, uint8_t val)
{
  uint8_t buff[2];
  buff[0] = reg;
  buff[1] = val;
  return writeToReg(I2C_ADDR_CEC, buff, 2, NULL, 0);
}
static Status sendToHDMI(uint8_t reg, uint8_t val)
{
  uint8_t buff[2];
  buff[0] = reg;
  buff[1] = val;
  return writeToReg(I2C_ADDR_HDMI, buff, 2, NULL, 0);
}
static Status send16ToHDMI(uint8_t reg, uint16_t val)
{
  uint8_t buff[2];
  buff[0] = reg;
  buff[1] = val >> 8;
  if (writeToReg(I2C_ADDR_HDMI, buff, 2, NULL, 0) == ERROR)
    return ERROR;
  buff[0] = reg+1;
  buff[1] = val & 0x00ff;
  return writeToReg(I2C_ADDR_HDMI, buff, 2, NULL, 0);
}

static Status setupSVGA(void)
{
  Status result = ERROR;
  uint8_t resolution = 0x00;
  
  uint16_t refpix = 229;
  uint16_t refline = 25;
  uint16_t npix = 1022;
  uint16_t nline = 624;
  uint16_t vs_line_start_1= 1;
  uint16_t vs_pix_start_1 = 24;
  uint16_t vs_line_end_1 = 3;
  uint16_t vs_pix_end_1 = 24;
  uint16_t hs_pix_start = 24;
  uint16_t vwin_start_1 = 24;
  uint16_t vwin_end_1 = 624;
  uint16_t de_start = 224;
  uint16_t de_end = 1024;    
  

#define ASSERT(__x) if ((__x)==ERROR) { break; }
  do
  {  
    Board_DelayMs(10);

    //write register FF of CEC device to 0x02.
    //(68ff = 02h) -> Only activate HDMI (no CEC and no RXSense)
    //(68ff = 87h) -> Activate all clocks
    ASSERT(sendToCEC(0xff, 0x87))
    
    Board_DelayMs(50);

    //select page 00 of HDMI device :  write 00 in FF
    //(e0ff =00h)
    ASSERT(sendToHDMI(0xff, 0x00))

    //write register A0 of HDMI device to select predefined format :
    //(e0a0 = 00h)       -> select a predefined 640x480p @60Hz format
    ASSERT(sendToHDMI(0xa0, resolution))

    // Reference pixel preset
    ASSERT(send16ToHDMI(0xa1, refpix))
    
    // Reference line preset
    ASSERT(send16ToHDMI(0xa3, refline))

    // Number of pixel per line
    ASSERT(send16ToHDMI(0xa5, npix))
    
    // Number of lines per frame
    ASSERT(send16ToHDMI(0xa7, nline))//STRANGE???? vs_line_start_1
    
    // VS LINE number for start pulse in field 1
    ASSERT(send16ToHDMI(0xa9, vs_line_start_1))//STRANGE???? vs_pix_start_1
    
    // VS PIXEL number for start pulse in field 1 
    ASSERT(send16ToHDMI(0xab, vs_pix_start_1))//STRANGE????vs_line_end_1
 
    // VS LINE number for end pulse in field 1
    ASSERT(send16ToHDMI(0xad, vs_line_end_1))
 
    // VS PIXEL number for end pulse in field 1
    ASSERT(send16ToHDMI(0xaf, vs_pix_end_1))
 
    // HS PIXEL number for start pulse in field 1
    ASSERT(send16ToHDMI(0xb9, hs_pix_start))
 
    // VWIN LINE number for start pulse in field 1
    ASSERT(send16ToHDMI(0xbd, vwin_start_1))
 
    // VWIN LINE number for end pulse in field 1
    ASSERT(send16ToHDMI(0xbf, vwin_end_1))
 
    // DE pixel number for start pulse in field 1	
    ASSERT(send16ToHDMI(0xc5, de_start))
 
    // DE pixel number for start pulse in field 2
    ASSERT(send16ToHDMI(0xc7, de_end))
    
    
    //write register TBG_CTRL1 of HDMI device to select resync method
    //(e0cb = 00h)        -> select
    ASSERT(sendToHDMI(0xcb, 0x7c))

    //write registers VIP_CTRL to configure RGB input ports :
    //(e020 = 01h)       -> select VP port configuration
    //(e021 = 23h)
    //(e022 = 47h)
    //(e023 = 16h)
    ASSERT(sendToHDMI(0x20, 0x01)) //0x23)) //0x01))
    ASSERT(sendToHDMI(0x21, 0x23)) //0x45)) //0x23))
    ASSERT(sendToHDMI(0x22, 0x47)) //0x01)) //0x47))
    ASSERT(sendToHDMI(0x23, 0x16)) //0x20)) //0x16))
    
    //clear register E4 of HDMI device to prevent any test patterns
    //(e0e4 = 00h)  - No test pattern (normal usage)
    //(e0e4 = c0h)  - Display 8-bar test pattern
    ASSERT(sendToHDMI(0xe4, 0x00))
    //ASSERT(sendToHDMI(0xe4, 0xc0))
    
//     //write register F0 of HDMI device to select unrepeated HDMI output :
//     //(e0f0 = 00h)
//     ASSERT(sendToHDMI(0xf0, 0x00))
    
    //clear register E5 of HDMI device
    //(e0e5 = 10h)
    ASSERT(sendToHDMI(0xe5, 0x10))
    
    result = SUCCESS;
    
  } while (FALSE);
  
  return result;
}

/******************************************************************************
 * Public Functions
 *****************************************************************************/

/******************************************************************************
 *
 * Description:
 *    Initialize the TDA19988
 *
 * Return:
 *    SUCCESS or ERROR
 *
 *****************************************************************************/
Status tda19988_init(void)
{
  return setup(RES_VGA);
}

/******************************************************************************
 *
 * Description:
 *    Change resolution
 *
 * Return:
 *    SUCCESS or ERROR
 *
 *****************************************************************************/
Status tda19988_setRes(tda19988_res res)
{
  if (res == RES_SVGA)
    return setupSVGA();
  
  return setup(res);
}

