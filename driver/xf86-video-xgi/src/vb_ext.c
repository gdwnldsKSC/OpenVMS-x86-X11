/* Copyright (C) 2003-2006 by XGI Technology, Taiwan.
 *
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation on the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL XGI AND/OR
 *  ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
 
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "osdef.h"

#ifdef LINUX_XF86
#include "xf86.h"
#include "xf86PciInfo.h"
#include "xgi.h"
#include "xgi_regs.h"
#endif

#ifdef LINUX_KERNEL
#include <linux/version.h>
#include <asm/io.h>
#include <linux/types.h>
#include "XGIfb.h"
#define MASK_OUTPUTSTATE_CRT2LCD 0x02
#endif

#include "vb_def.h"
#include "vgatypes.h"
#include "vb_struct.h"
#include "vb_setmode.h"
#include "vb_ext.h"
#include "vb_init.h"

static int XGINew_GetLCDDDCInfo(PXGI_HW_DEVICE_INFO HwDeviceExtension,
    PVB_DEVICE_INFO pVBInfo);

static BOOLEAN XGINew_BridgeIsEnable(PXGI_HW_DEVICE_INFO,
    PVB_DEVICE_INFO pVBInfo);

static BOOLEAN XGINew_Sense(USHORT tempbx, USHORT tempcx,
			    PVB_DEVICE_INFO pVBInfo);

static BOOLEAN XGINew_SenseHiTV(PXGI_HW_DEVICE_INFO HwDeviceExtension,
				PVB_DEVICE_INFO pVBInfo);

/**************************************************************
	Dynamic Sense
*************************************************************/


/**
 * Determine if the CRTC is a 301B.
 */
static int XGINew_Is301B(PVB_DEVICE_INFO pVBInfo)
{
    return !(XGI_GetReg((XGIIOADDRESS)pVBInfo->Part4Port, 0x01) > 0x0B0);
}

/* --------------------------------------------------------------------- */
/* Function : XGI_Is301C */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
BOOLEAN XGI_Is301C( PVB_DEVICE_INFO pVBInfo )
{
    if ( ( XGI_GetReg( (XGIIOADDRESS) pVBInfo->Part4Port , 0x01 ) & 0xF0 ) == 0xC0 )
        return( 1 ) ;

    if ( XGI_GetReg( (XGIIOADDRESS) pVBInfo->Part4Port , 0x01 ) >= 0xD0 )
    {
        if ( XGI_GetReg( (XGIIOADDRESS) pVBInfo->Part4Port , 0x39 ) == 0xE0 )
            return( 1 ) ;
    }

    return( 0 ) ;
}

/* --------------------------------------------------------------------- */
/* Function : XGINew_Sense */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
BOOLEAN XGINew_Sense(USHORT tempbx, USHORT tempcx, PVB_DEVICE_INFO pVBInfo)
{
    unsigned temp;
    unsigned i;


    XGI_SetReg((XGIIOADDRESS)pVBInfo->Part4Port, 0x11, (tempbx & 0x0FF));

    XGI_SetRegANDOR((XGIIOADDRESS)pVBInfo->Part4Port, 0x10, ~0x1F, 
		       ((tempbx & 0xFF00) >> 8) | (tempcx & 0x00FF));

    for (i = 0; i < 10; i++)
        XGI_WaitEndRetrace(pVBInfo->RelIO);

    temp = XGI_GetReg((XGIIOADDRESS) pVBInfo->Part4Port, 0x03);
    temp = (temp ^ 0x0E) & ((tempcx & 0x7F00) >> 8);

    return (temp > 0);
}


/* --------------------------------------------------------------------- */
/* Function : XGI_GetSenseStatus */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void XGI_GetSenseStatus( PXGI_HW_DEVICE_INFO HwDeviceExtension , PVB_DEVICE_INFO pVBInfo)
{
    USHORT tempax = 0 , tempbx , tempcx , temp ,
           P2reg0 = 0 , SenseModeNo = 0 , OutputSelect = pVBInfo->OutputSelect,
           ModeIdIndex , i ;
    pVBInfo->BaseAddr = ( USHORT )HwDeviceExtension->pjIOAddress ;

    {		/* for 301 */
        if ( pVBInfo->VBInfo & SetCRT2ToHiVisionTV )
        {	/* for HiVision */
            tempax = XGI_GetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x38 ) ;
            temp = tempax & 0x01 ;
            tempax = XGI_GetReg((XGIIOADDRESS) pVBInfo->P3c4 , 0x3A ) ;
            temp = temp | ( tempax & 0x02 ) ;
            XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->P3d4 , 0x32 , 0xA0 , temp ) ;
        }
        else
        {
            if ( XGI_BridgeIsOn( pVBInfo ) )
            {
                P2reg0 = XGI_GetReg((XGIIOADDRESS) pVBInfo->Part2Port , 0x00 ) ;
                if ( !XGINew_BridgeIsEnable( HwDeviceExtension, pVBInfo ) )
                {
		    USHORT temp_mode_no;

                    SenseModeNo = 0x2e ;
                    /* XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4 , 0x30 , 0x41 ) ; */
		    /* XGISetModeNew(HwDeviceExtension, pVBInfo, 0x2e); // ynlai InitMode */

		    temp_mode_no = SenseModeNo;
		    XGI_SearchModeID(pVBInfo->SModeIDTable,
				     pVBInfo->EModeIDTable, 0x11,
				     &temp_mode_no, &ModeIdIndex);

                    XGI_GetVBType( pVBInfo ) ;
                    pVBInfo->SetFlag = 0x00 ;
                    pVBInfo->ModeType = ModeVGA ;
                    pVBInfo->VBInfo = SetCRT2ToRAMDAC | LoadDACFlag | SetInSlaveMode ;
                    XGI_GetLCDInfo( 0x2e , ModeIdIndex, pVBInfo ) ;
                    XGI_GetTVInfo(  0x2e , ModeIdIndex, pVBInfo ) ;
                    XGI_EnableBridge( HwDeviceExtension, pVBInfo ) ;
                    XGI_SetCRT2Group301( SenseModeNo , HwDeviceExtension, pVBInfo ) ;
                    XGI_SetCRT2ModeRegs( 0x2e , HwDeviceExtension, pVBInfo ) ;
                    /* XGI_DisableBridge( HwDeviceExtension, pVBInfo ) ; */
                    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->P3c4 , 0x01 , 0xDF , 0x20 ) ;	/* Display Off 0212 */
                    for( i = 0 ; i < 20 ; i++ )
                    {
                        XGI_WaitEndRetrace(pVBInfo->RelIO) ;
                    }
                }
                XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port , 0x00 , 0x1c ) ;
                tempax = 0 ;
                tempbx = *pVBInfo->pRGBSenseData ;

                if ( !( XGINew_Is301B( pVBInfo ) ) )
                {
                    tempbx = *pVBInfo->pRGBSenseData2 ;
                }

                tempcx = 0x0E08 ;
                if (XGINew_Sense(tempbx, tempcx, pVBInfo)) {
                    if (XGINew_Sense(tempbx, tempcx, pVBInfo)) {
                        tempax |= Monitor2Sense;
                    }
                }

                if ( pVBInfo->VBType & VB_XGI301C)
                {
                    XGI_SetRegOR((XGIIOADDRESS) pVBInfo->Part4Port , 0x0d , 0x04 ) ;
                }

		if ( XGINew_SenseHiTV( HwDeviceExtension , pVBInfo) )		/* add by kuku for Multi-adapter sense HiTV */
		{
		    tempax |= HiTVSense ;
                    if ( ( pVBInfo->VBType & VB_XGI301C ) )
                    {
                	tempax ^= ( HiTVSense | YPbPrSense ) ;
                    }
                }

		if ( !( tempax & ( HiTVSense | YPbPrSense ) ) )		/* start */
                {

                tempbx = *pVBInfo->pYCSenseData ;

                if ( !( XGINew_Is301B(  pVBInfo ) ) )
                {
                    tempbx=*pVBInfo->pYCSenseData2;
                }

                tempcx = 0x0604 ;
                if (XGINew_Sense(tempbx, tempcx, pVBInfo)) {
                    if (XGINew_Sense(tempbx, tempcx, pVBInfo)) {
                        tempax |= SVIDEOSense ;
                    }
                }

                if ( OutputSelect & BoardTVType )
                {
                    tempbx = *pVBInfo->pVideoSenseData ;

                    if ( !( XGINew_Is301B( pVBInfo ) ) )
                    {
                        tempbx = *pVBInfo->pVideoSenseData2 ;
                    }

                    tempcx = 0x0804 ;
                    if (XGINew_Sense(tempbx, tempcx, pVBInfo)) {
                        if (XGINew_Sense(tempbx, tempcx, pVBInfo)) {
                            tempax |= AVIDEOSense;
                        }
                    }
                }
                else
                {
                    if ( !( tempax & SVIDEOSense ) )
                    {
                        tempbx = *pVBInfo->pVideoSenseData ;

                        if ( !( XGINew_Is301B( pVBInfo ) ) )
                        {
                            tempbx=*pVBInfo->pVideoSenseData2;
                        }

                        tempcx = 0x0804 ;
                        if (XGINew_Sense(tempbx, tempcx, pVBInfo)) {
                            if (XGINew_Sense(tempbx, tempcx, pVBInfo)) {
                                tempax |= AVIDEOSense;
                            }
                        }
                    }
                }
            }
    	    } /* end */
            if ( !( tempax & Monitor2Sense ) )
            {
                if (XGINew_GetLCDDDCInfo(HwDeviceExtension, pVBInfo)) {
                    tempax |= LCDSense;
                }
            }
            tempbx = 0 ;
            tempcx = 0 ;
            XGINew_Sense(tempbx, tempcx, pVBInfo);

            XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->P3d4 , 0x32 , ~0xDF , tempax ) ;
            XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port , 0x00 , P2reg0 ) ;

            if ( !( P2reg0 & 0x20 ) )
            {
                pVBInfo->VBInfo = DisableCRT2Display ;
                /* XGI_SetCRT2Group301( SenseModeNo , HwDeviceExtension, pVBInfo ) ; */
            }
        }
    }
    XGI_DisableBridge( HwDeviceExtension, pVBInfo ) ;		/* shampoo 0226 */
}



/* --------------------------------------------------------------------- */
/* Function : XGINew_GetLCDDDCInfo */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
int
XGINew_GetLCDDDCInfo(PXGI_HW_DEVICE_INFO HwDeviceExtension,
		     PVB_DEVICE_INFO pVBInfo)
{
    unsigned temp = HwDeviceExtension->ulCRT2LCDType;

    switch (HwDeviceExtension->ulCRT2LCDType) {
    /* add lcd sense */
    case LCD_UNKNOWN:
	return 0;

    case LCD_INVALID:
    case LCD_800x600:
    case LCD_1024x768:
    case LCD_1280x1024:
	break;

    case LCD_640x480:
    case LCD_1024x600:
    case LCD_1152x864:
    case LCD_1280x960:
    case LCD_1152x768:
	temp = 0;
	break;

    case LCD_1400x1050:
    case LCD_1280x768:
    case LCD_1600x1200:
	break;

    case LCD_1920x1440:
    case LCD_2048x1536:
	temp = 0;
	break;

    default:
	break;
    }

    XGI_SetRegANDOR((XGIIOADDRESS)pVBInfo->P3d4, 0x36, 0xF0, temp);
    return 1 ;
}


/* --------------------------------------------------------------------- */
/* Function : XGINew_BridgeIsEnable */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
BOOLEAN XGINew_BridgeIsEnable( PXGI_HW_DEVICE_INFO HwDeviceExtension ,PVB_DEVICE_INFO pVBInfo)
{
    USHORT flag ;

    if ( XGI_BridgeIsOn( pVBInfo ) == 0 )
    {
        flag = XGI_GetReg((XGIIOADDRESS) pVBInfo->Part1Port , 0x0 ) ;

        if ( flag & 0x050 )
        {
            return( 1 ) ;
        }
        else
        {
            return( 0 ) ;
        }

    }
    return( 0 ) ;
}

/* ------------------------------------------------------ */
/* Function : XGINew_SenseHiTV */
/* Input : */
/* Output : */
/* Description : */
/* ------------------------------------------------------ */
BOOLEAN XGINew_SenseHiTV( PXGI_HW_DEVICE_INFO HwDeviceExtension , PVB_DEVICE_INFO pVBInfo )
{
    USHORT tempbx , tempcx , temp , i , tempch;

#ifndef LINUX_XF86
    USHORT tempax ;
#endif
    tempbx = *pVBInfo->pYCSenseData2 ;

    tempcx = 0x0604 ;

    temp = tempbx & 0xFF ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part4Port , 0x11 , temp ) ;
    temp = ( tempbx & 0xFF00 ) >> 8 ;
    temp |= ( tempcx & 0x00FF ) ;
    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part4Port , 0x10 , ~0x1F , temp ) ;

    for( i = 0 ; i < 10 ; i++ )
        XGI_WaitEndRetrace(pVBInfo->RelIO) ;

    tempch = ( tempcx & 0xFF00 ) >> 8;
    temp = XGI_GetReg((XGIIOADDRESS) pVBInfo->Part4Port , 0x03 ) ;
    temp = temp ^ ( 0x0E ) ;
    temp &= tempch ;

    if ( temp !=  tempch )
        return( 0 ) ;

    tempbx = *pVBInfo->pVideoSenseData2 ;

    tempcx = 0x0804 ;
    temp = tempbx & 0xFF ;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part4Port , 0x11 , temp ) ;
    temp = ( tempbx & 0xFF00 ) >> 8 ;
    temp |= ( tempcx & 0x00FF ) ;
    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part4Port , 0x10 , ~0x1F , temp ) ;

    for( i = 0 ; i < 10 ; i++ )
        XGI_WaitEndRetrace(pVBInfo->RelIO) ;

    tempch = ( tempcx & 0xFF00 ) >> 8;
    temp = XGI_GetReg((XGIIOADDRESS) pVBInfo->Part4Port , 0x03 ) ;
    temp = temp ^ ( 0x0E ) ;
    temp &= tempch ;

    if ( temp !=  tempch )
        return( 0 ) ;
    else
    {
      tempbx = 0x3FF ;
      tempcx = 0x0804 ;
      temp = tempbx & 0xFF ;
      XGI_SetReg((XGIIOADDRESS) pVBInfo->Part4Port , 0x11 , temp ) ;
      temp = ( tempbx & 0xFF00 ) >> 8 ;
      temp |= ( tempcx & 0x00FF ) ;
      XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part4Port , 0x10 , ~0x1F , temp ) ;

      for( i = 0 ; i < 10 ; i++ )
          XGI_WaitEndRetrace(pVBInfo->RelIO) ;

      tempch = ( tempcx & 0xFF00 ) >> 8;
      temp = XGI_GetReg((XGIIOADDRESS) pVBInfo->Part4Port , 0x03 ) ;
      temp = temp ^ ( 0x0E ) ;
      temp &= tempch ;

      if ( temp != tempch )
          return( 1 ) ;
      else
	  return( 0 ) ;
    }
}
