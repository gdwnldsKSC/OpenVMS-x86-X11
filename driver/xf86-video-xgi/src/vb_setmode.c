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
#include <asm/io.h>
#include <linux/types.h>
#include <linux/version.h>
#include "XGIfb.h"
#endif

#include "vb_def.h"
#include "vgatypes.h"
#include "vb_struct.h"
#include "vb_table.h"
#include "vb_setmode.h"

#define  IndexMask 0xff
#ifndef XGI_MASK_DUAL_CHIP
#define XGI_MASK_DUAL_CHIP	  0x04  /* SR3A */
#endif


BOOLEAN CheckDualChip(PVB_DEVICE_INFO pVBInfo);
static BOOLEAN XGI_IsLCDDualLink(PVB_DEVICE_INFO pVBInfo);
BOOLEAN XGI_SetCRT2Group301(USHORT ModeNo,
                            PXGI_HW_DEVICE_INFO HwDeviceExtension,
                            PVB_DEVICE_INFO pVBInfo);
BOOLEAN XGI_BacklightByDrv(PVB_DEVICE_INFO pVBInfo);

BOOLEAN XGI_IsLCDON(PVB_DEVICE_INFO pVBInfo);
BOOLEAN XGI_DisableChISLCD(PVB_DEVICE_INFO pVBInfo);
BOOLEAN XGI_EnableChISLCD(PVB_DEVICE_INFO pVBInfo);
BOOLEAN XGI_AjustCRT2Rate(USHORT ModeNo, USHORT ModeIdIndex,
                          USHORT RefreshRateTableIndex, USHORT * i,
                          PVB_DEVICE_INFO pVBInfo);
BOOLEAN XGI_GetLCDInfo(USHORT ModeNo, USHORT ModeIdIndex,
                       PVB_DEVICE_INFO pVBInfo);
BOOLEAN XGI_BridgeIsOn(PVB_DEVICE_INFO pVBInfo);
USHORT XGI_GetOffset(USHORT ModeNo, USHORT ModeIdIndex,
                     USHORT RefreshRateTableIndex,
                     PXGI_HW_DEVICE_INFO HwDeviceExtension,
                     PVB_DEVICE_INFO pVBInfo);
USHORT XGI_GetRatePtrCRT2(USHORT ModeNo, USHORT ModeIdIndex,
                          PVB_DEVICE_INFO pVBInfo);
static USHORT XGI_GetResInfo(USHORT ModeNo, USHORT ModeIdIndex,
                             PVB_DEVICE_INFO pVBInfo);
USHORT XGI_GetVGAHT2(PVB_DEVICE_INFO pVBInfo);
static unsigned XGI_GetVCLK2Ptr(USHORT ModeNo, USHORT ModeIdIndex,
                                USHORT RefreshRateTableIndex,
                                PVB_DEVICE_INFO pVBInfo);
void XGI_VBLongWait(PVB_DEVICE_INFO pVBInfo);
void XGI_SaveCRT2Info(USHORT ModeNo, PVB_DEVICE_INFO pVBInfo);
void XGI_GetCRT2Data(USHORT ModeNo, USHORT ModeIdIndex,
                     USHORT RefreshRateTableIndex, PVB_DEVICE_INFO pVBInfo);
void XGI_GetCRT2ResInfo(USHORT ModeNo, USHORT ModeIdIndex,
                        PVB_DEVICE_INFO pVBInfo);
void XGI_PreSetGroup1(USHORT ModeNo, USHORT ModeIdIndex,
                      PXGI_HW_DEVICE_INFO HwDeviceExtension,
                      USHORT RefreshRateTableIndex, PVB_DEVICE_INFO pVBInfo);
void XGI_SetGroup1(USHORT ModeNo, USHORT ModeIdIndex,
                   PXGI_HW_DEVICE_INFO HwDeviceExtension,
                   USHORT RefreshRateTableIndex, PVB_DEVICE_INFO pVBInfo);
void XGI_SetLockRegs(USHORT ModeNo, USHORT ModeIdIndex,
                     PXGI_HW_DEVICE_INFO HwDeviceExtension,
                     USHORT RefreshRateTableIndex, PVB_DEVICE_INFO pVBInfo);
void XGI_SetLCDRegs(USHORT ModeNo, USHORT ModeIdIndex,
                    PXGI_HW_DEVICE_INFO HwDeviceExtension,
                    USHORT RefreshRateTableIndex, PVB_DEVICE_INFO pVBInfo);
void XGI_SetGroup2(USHORT ModeNo, USHORT ModeIdIndex,
                   USHORT RefreshRateTableIndex,
                   PXGI_HW_DEVICE_INFO HwDeviceExtension,
                   PVB_DEVICE_INFO pVBInfo);
void XGI_SetGroup3(USHORT ModeNo, USHORT ModeIdIndex,
                   PVB_DEVICE_INFO pVBInfo);
void XGI_SetGroup4(USHORT ModeNo, USHORT ModeIdIndex,
                   USHORT RefreshRateTableIndex,
                   PXGI_HW_DEVICE_INFO HwDeviceExtension,
                   PVB_DEVICE_INFO pVBInfo);
void XGI_SetGroup5(USHORT ModeNo, USHORT ModeIdIndex,
                   PVB_DEVICE_INFO pVBInfo);
static const void *XGI_GetLcdPtr(USHORT BX, USHORT ModeNo, USHORT ModeIdIndex,
                    USHORT RefreshRateTableIndex, PVB_DEVICE_INFO pVBInfo);
static const void *XGI_GetTVPtr(USHORT BX, USHORT ModeNo, USHORT ModeIdIndex,
                   USHORT RefreshRateTableIndex, PVB_DEVICE_INFO pVBInfo);
void XGI_FirePWDEnable(PVB_DEVICE_INFO pVBInfo);
void XGI_EnableGatingCRT(PXGI_HW_DEVICE_INFO HwDeviceExtension,
                         PVB_DEVICE_INFO pVBInfo);
void XGI_DisableGatingCRT(PXGI_HW_DEVICE_INFO HwDeviceExtension,
                          PVB_DEVICE_INFO pVBInfo);
void XGI_SetPanelDelay(USHORT tempbl, PVB_DEVICE_INFO pVBInfo);
void XGI_SetPanelPower(USHORT tempah, USHORT tempbl, PVB_DEVICE_INFO pVBInfo);
void XGI_EnablePWD(PVB_DEVICE_INFO pVBInfo);
void XGI_DisablePWD(PVB_DEVICE_INFO pVBInfo);
void XGI_AutoThreshold(PVB_DEVICE_INFO pVBInfo);
void XGI_SetTap4Regs(PVB_DEVICE_INFO pVBInfo);
void SetDualChipRegs(PXGI_HW_DEVICE_INFO, PVB_DEVICE_INFO pVBInfo);
void XGI_DisplayOn(PVB_DEVICE_INFO pVBInfo);
void XGI_DisplayOff(PVB_DEVICE_INFO pVBInfo);
void XGI_SetCRT1Group(PXGI_HW_DEVICE_INFO HwDeviceExtension, USHORT ModeNo,
                      USHORT ModeIdIndex, PVB_DEVICE_INFO pVBInfo);
static void XGI_WaitDisplay(PVB_DEVICE_INFO pVBInfo);
void XGI_SenseCRT1(PVB_DEVICE_INFO pVBInfo);

void XGI_SetCRT1CRTC(USHORT ModeNo, USHORT ModeIdIndex,
                     USHORT RefreshRateTableIndex, PVB_DEVICE_INFO pVBInfo,
                     PXGI_HW_DEVICE_INFO HwDeviceExtension);
void XGI_SetCRT1Timing_H(PVB_DEVICE_INFO pVBInfo,
                         PXGI_HW_DEVICE_INFO HwDeviceExtension);
void XGI_SetCRT1Timing_V(USHORT ModeIdIndex, USHORT ModeNo,
                         PVB_DEVICE_INFO pVBInfo);
void XGI_SetCRT1DE(PXGI_HW_DEVICE_INFO HwDeviceExtension, USHORT ModeNo,
                   USHORT ModeIdIndex, USHORT RefreshRateTableIndex,
                   PVB_DEVICE_INFO pVBInfo);
void XGI_SetCRT1VCLK(USHORT ModeNo, USHORT ModeIdIndex,
                     PXGI_HW_DEVICE_INFO HwDeviceExtension,
                     USHORT RefreshRateTableIndex, PVB_DEVICE_INFO pVBInfo);
void XGI_SetCRT1FIFO(USHORT ModeNo, PXGI_HW_DEVICE_INFO HwDeviceExtension,
                     PVB_DEVICE_INFO pVBInfo);
void XGI_SetCRT1ModeRegs(PXGI_HW_DEVICE_INFO HwDeviceExtension, USHORT ModeNo,
                         USHORT ModeIdIndex, USHORT RefreshRateTableIndex,
                         PVB_DEVICE_INFO pVBInfo);
void XGI_SetVCLKState(PXGI_HW_DEVICE_INFO HwDeviceExtension, USHORT ModeNo,
                      USHORT RefreshRateTableIndex, PVB_DEVICE_INFO pVBInfo);

void XGI_LoadDAC(USHORT ModeNo, USHORT ModeIdIndex, PVB_DEVICE_INFO pVBInfo);
void XGI_SetLCDAGroup(USHORT ModeNo, USHORT ModeIdIndex,
                      PXGI_HW_DEVICE_INFO HwDeviceExtension,
                      PVB_DEVICE_INFO pVBInfo);
void XGI_GetLVDSResInfo(USHORT ModeNo, USHORT ModeIdIndex,
                        PVB_DEVICE_INFO pVBInfo);
void XGI_GetLVDSData(USHORT ModeNo, USHORT ModeIdIndex,
                     USHORT RefreshRateTableIndex, PVB_DEVICE_INFO pVBInfo);
void XGI_ModCRT1Regs(USHORT ModeNo, USHORT ModeIdIndex,
                     USHORT RefreshRateTableIndex,
                     PXGI_HW_DEVICE_INFO HwDeviceExtension,
                     PVB_DEVICE_INFO pVBInfo);
void XGI_SetLVDSRegs(USHORT ModeNo, USHORT ModeIdIndex,
                     USHORT RefreshRateTableIndex, PVB_DEVICE_INFO pVBInfo);
void XGI_UpdateModeInfo(PXGI_HW_DEVICE_INFO HwDeviceExtension,
                        PVB_DEVICE_INFO pVBInfo);
void XGI_GetVBType(PVB_DEVICE_INFO pVBInfo);
void XGI_GetVBInfo(USHORT ModeNo, USHORT ModeIdIndex,
                   PXGI_HW_DEVICE_INFO HwDeviceExtension,
                   PVB_DEVICE_INFO pVBInfo);
void XGI_GetTVInfo(USHORT ModeNo, USHORT ModeIdIndex,
                   PVB_DEVICE_INFO pVBInfo);
void XGI_SetCRT2ECLK(USHORT ModeNo, USHORT ModeIdIndex,
                     USHORT RefreshRateTableIndex, PVB_DEVICE_INFO pVBInfo);
void InitTo330Pointer(UCHAR, PVB_DEVICE_INFO pVBInfo);
void XGI_GetLCDSync(USHORT * HSyncWidth, USHORT * VSyncWidth,
                    PVB_DEVICE_INFO pVBInfo);
void XGI_DisableBridge(PXGI_HW_DEVICE_INFO HwDeviceExtension,
                       PVB_DEVICE_INFO pVBInfo);
void XGI_EnableBridge(PXGI_HW_DEVICE_INFO HwDeviceExtension,
                      PVB_DEVICE_INFO pVBInfo);
void XGI_SetCRT2VCLK(USHORT ModeNo, USHORT ModeIdIndex,
                     USHORT RefreshRateTableIndex, PVB_DEVICE_INFO pVBInfo);
void XGI_OEM310Setting(USHORT ModeNo, USHORT ModeIdIndex,
                       PVB_DEVICE_INFO pVBInfo);
void XGI_SetDelayComp(PVB_DEVICE_INFO pVBInfo);
void XGI_SetLCDCap(PVB_DEVICE_INFO pVBInfo);
void XGI_SetLCDCap_A(USHORT tempcx, PVB_DEVICE_INFO pVBInfo);
void XGI_SetLCDCap_B(USHORT tempcx, PVB_DEVICE_INFO pVBInfo);
void SetSpectrum(PVB_DEVICE_INFO pVBInfo);
void XGI_SetAntiFlicker(USHORT ModeNo, USHORT ModeIdIndex,
                        PVB_DEVICE_INFO pVBInfo);
void XGI_SetEdgeEnhance(USHORT ModeNo, USHORT ModeIdIndex,
                        PVB_DEVICE_INFO pVBInfo);
void XGI_SetPhaseIncr(PVB_DEVICE_INFO pVBInfo);
void XGI_SetYFilter(USHORT ModeNo, USHORT ModeIdIndex,
                    PVB_DEVICE_INFO pVBInfo);
void XGI_GetTVPtrIndex2(USHORT * tempbx, UCHAR * tempcl, UCHAR * tempch,
                        PVB_DEVICE_INFO pVBInfo);
USHORT XGI_GetTVPtrIndex(PVB_DEVICE_INFO pVBInfo);
void XGI_SetCRT2ModeRegs(USHORT ModeNo, PXGI_HW_DEVICE_INFO,
                         PVB_DEVICE_INFO pVBInfo);
void XGI_GetRAMDAC2DATA(USHORT ModeNo, USHORT ModeIdIndex,
                        USHORT RefreshRateTableIndex,
                        PVB_DEVICE_INFO pVBInfo);
void XGI_UnLockCRT2(PXGI_HW_DEVICE_INFO, PVB_DEVICE_INFO pVBInfo);
void XGI_LockCRT2(PXGI_HW_DEVICE_INFO, PVB_DEVICE_INFO pVBInfo);
void XGINew_EnableCRT2(PVB_DEVICE_INFO pVBInfo);
void XGINew_LCD_Wait_Time(UCHAR DelayTime, PVB_DEVICE_INFO pVBInfo);
void XGI_SetCRT1Offset(USHORT ModeNo, USHORT ModeIdIndex,
                       USHORT RefreshRateTableIndex,
                       PXGI_HW_DEVICE_INFO HwDeviceExtension,
                       PVB_DEVICE_INFO pVBInfo);
static void XGI_GetLCDVCLKPtr(UCHAR *di, PVB_DEVICE_INFO pVBInfo);
static unsigned XGI_GetVCLKPtr(USHORT RefreshRateTableIndex, USHORT ModeNo,
                               USHORT ModeIdIndex, PVB_DEVICE_INFO pVBInfo);
static void XGI_GetVCLKLen(unsigned vclkindex, UCHAR *di,
                           PVB_DEVICE_INFO pVBInfo);
USHORT XGI_GetLCDCapPtr(PVB_DEVICE_INFO pVBInfo);
USHORT XGI_GetLCDCapPtr1(PVB_DEVICE_INFO pVBInfo);
static const XGI301C_Tap4TimingStruct *XGI_GetTap4Ptr(USHORT tempcx,
                                                      PVB_DEVICE_INFO pVBInfo);


const uint8_t XGI_MDA_DAC[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15,
    0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15,
    0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15,
    0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15,
    0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F
};

const uint8_t XGI_CGA_DAC[] = {
    0x00, 0x10, 0x04, 0x14, 0x01, 0x11, 0x09, 0x15,
    0x00, 0x10, 0x04, 0x14, 0x01, 0x11, 0x09, 0x15,
    0x2A, 0x3A, 0x2E, 0x3E, 0x2B, 0x3B, 0x2F, 0x3F,
    0x2A, 0x3A, 0x2E, 0x3E, 0x2B, 0x3B, 0x2F, 0x3F,
    0x00, 0x10, 0x04, 0x14, 0x01, 0x11, 0x09, 0x15,
    0x00, 0x10, 0x04, 0x14, 0x01, 0x11, 0x09, 0x15,
    0x2A, 0x3A, 0x2E, 0x3E, 0x2B, 0x3B, 0x2F, 0x3F,
    0x2A, 0x3A, 0x2E, 0x3E, 0x2B, 0x3B, 0x2F, 0x3F
};

const uint8_t XGI_EGA_DAC[] = {
    0x00, 0x10, 0x04, 0x14, 0x01, 0x11, 0x05, 0x15,
    0x20, 0x30, 0x24, 0x34, 0x21, 0x31, 0x25, 0x35,
    0x08, 0x18, 0x0C, 0x1C, 0x09, 0x19, 0x0D, 0x1D,
    0x28, 0x38, 0x2C, 0x3C, 0x29, 0x39, 0x2D, 0x3D,
    0x02, 0x12, 0x06, 0x16, 0x03, 0x13, 0x07, 0x17,
    0x22, 0x32, 0x26, 0x36, 0x23, 0x33, 0x27, 0x37,
    0x0A, 0x1A, 0x0E, 0x1E, 0x0B, 0x1B, 0x0F, 0x1F,
    0x2A, 0x3A, 0x2E, 0x3E, 0x2B, 0x3B, 0x2F, 0x3F
};

const uint8_t XGI_VGA_DAC[] = {
    0x00, 0x10, 0x04, 0x14, 0x01, 0x11, 0x09, 0x15,
    0x2A, 0x3A, 0x2E, 0x3E, 0x2B, 0x3B, 0x2F, 0x3F,
    0x00, 0x05, 0x08, 0x0B, 0x0E, 0x11, 0x14, 0x18,
    0x1C, 0x20, 0x24, 0x28, 0x2D, 0x32, 0x38, 0x3F,
    0x00, 0x10, 0x1F, 0x2F, 0x3F, 0x1F, 0x27, 0x2F,
    0x37, 0x3F, 0x2D, 0x31, 0x36, 0x3A, 0x3F, 0x00,
    0x07, 0x0E, 0x15, 0x1C, 0x0E, 0x11, 0x15, 0x18,
    0x1C, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x00, 0x04,
    0x08, 0x0C, 0x10, 0x08, 0x0A, 0x0C, 0x0E, 0x10,
    0x0B, 0x0C, 0x0D, 0x0F, 0x10
};


/* --------------------------------------------------------------------- */
/* Function : InitTo330Pointer */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
InitTo330Pointer(UCHAR ChipType, PVB_DEVICE_INFO pVBInfo)
{
    pVBInfo->SModeIDTable = XGI330_SModeIDTable;
    pVBInfo->StandTable = XGI330_StandTable;
    pVBInfo->EModeIDTable = XGI330_EModeIDTable;
    pVBInfo->RefIndex = XGI330_RefIndex;
    pVBInfo->XGINEWUB_CRT1Table = XGI_CRT1Table;

    /* add for new UNIVGABIOS */
    /* XGINew_UBLCDDataTable = (XGI_LCDDataTablStruct *) XGI_LCDDataTable ; */
    /* XGINew_UBTVDataTable = (XGI_TVDataTablStruct *) XGI_TVDataTable ; */


    if (ChipType >= XG40) {
        (void) memcpy(pVBInfo->MCLKData, XGI340New_MCLKData, sizeof(XGI340New_MCLKData));
        (void) memcpy(pVBInfo->ECLKData, XGI340_ECLKData, sizeof(XGI340_ECLKData));
    }
    else {
        (void) memcpy(pVBInfo->MCLKData, XGI330New_MCLKData, sizeof(XGI330New_MCLKData));
        (void) memcpy(pVBInfo->ECLKData, XGI330_ECLKData, sizeof(XGI330_ECLKData));
    }

    pVBInfo->VCLKData = XGI_VCLKData;
    pVBInfo->VBVCLKData = XGI_VBVCLKData;
    pVBInfo->ScreenOffset = XGI330_ScreenOffset;
    pVBInfo->StResInfo = XGI330_StResInfo;
    pVBInfo->ModeResInfo = XGI330_ModeResInfo;

    pVBInfo->OutputSelect = XGI330_OutputSelect;
    pVBInfo->SoftSetting = XGI330_SoftSetting;
    pVBInfo->SR07 = XGI330_SR07;
    pVBInfo->LCDResInfo = 0;
    pVBInfo->LCDTypeInfo = 0;
    pVBInfo->LCDInfo = 0;
    pVBInfo->VBInfo = 0;
    pVBInfo->TVInfo = 0;


    (void) memcpy(pVBInfo->SR15, XGI340_SR13, sizeof(XGI340_SR13));
    (void) memcpy(pVBInfo->CR40, XGI340_CR41, sizeof(XGI340_CR41));
    (void) memcpy(pVBInfo->SR25, XGI330_SR25, sizeof(XGI330_SR25));
    pVBInfo->SR31 = XGI330_SR31;
    pVBInfo->SR32 = XGI330_SR32;
    (void) memcpy(pVBInfo->CR6B, XGI340_CR6B, sizeof(XGI340_CR6B));
    if (ChipType == XG45) {
        (void) memcpy(pVBInfo->XG45CR6E, XGI45_CR6E, sizeof(XGI45_CR6E));
        (void) memcpy(pVBInfo->XG45CR6F, XGI45_CR6F, sizeof(XGI45_CR6F));
    }
    else {
        (void) memcpy(pVBInfo->CR6E, XGI340_CR6E, sizeof(XGI340_CR6E));
        (void) memcpy(pVBInfo->CR6F, XGI340_CR6F, sizeof(XGI340_CR6F));
    }
    (void) memcpy(pVBInfo->CR89, XGI340_CR89, sizeof(XGI340_CR89));
    (void) memcpy(pVBInfo->AGPReg, XGI340_AGPReg, sizeof(XGI340_AGPReg));
    (void) memcpy(pVBInfo->SR16, XGI340_SR16, sizeof(XGI340_SR16));
    pVBInfo->CRCF = XG40_CRCF;
    pVBInfo->DRAMTypeDefinition = XG40_DRAMTypeDefinition;


    (void) memcpy(pVBInfo->CR49, XGI330_CR49, sizeof(XGI330_CR49));
    pVBInfo->SR1F = XGI330_SR1F;
    pVBInfo->SR21 = XGI330_SR21;
    pVBInfo->SR22 = XGI330_SR22;
    pVBInfo->SR23 = XGI330_SR23;
    pVBInfo->SR24 = XGI330_SR24;
    pVBInfo->SR33 = XGI330_SR33;



    pVBInfo->CRT2Data_1_2 = XGI330_CRT2Data_1_2;
    pVBInfo->CRT2Data_4_D = XGI330_CRT2Data_4_D;
    pVBInfo->CRT2Data_4_E = XGI330_CRT2Data_4_E;
    pVBInfo->CRT2Data_4_10 = XGI330_CRT2Data_4_10;
    pVBInfo->pRGBSenseData = &XGI330_RGBSenseData;
    pVBInfo->pVideoSenseData = &XGI330_VideoSenseData;
    pVBInfo->pYCSenseData = &XGI330_YCSenseData;
    pVBInfo->pRGBSenseData2 = &XGI330_RGBSenseData2;
    pVBInfo->pVideoSenseData2 = &XGI330_VideoSenseData2;
    pVBInfo->pYCSenseData2 = &XGI330_YCSenseData2;

    pVBInfo->NTSCTiming = XGI330_NTSCTiming;
    pVBInfo->PALTiming = XGI330_PALTiming;
    pVBInfo->HiTVExtTiming = XGI330_HiTVExtTiming;
    pVBInfo->HiTVSt1Timing = XGI330_HiTVSt1Timing;
    pVBInfo->HiTVSt2Timing = XGI330_HiTVSt2Timing;
    pVBInfo->HiTVTextTiming = XGI330_HiTVTextTiming;
    pVBInfo->YPbPr750pTiming = XGI330_YPbPr750pTiming;
    pVBInfo->YPbPr525pTiming = XGI330_YPbPr525pTiming;
    pVBInfo->YPbPr525iTiming = XGI330_YPbPr525iTiming;
    pVBInfo->HiTVGroup3Data = XGI330_HiTVGroup3Data;
    pVBInfo->HiTVGroup3Simu = XGI330_HiTVGroup3Simu;
    pVBInfo->HiTVGroup3Text = XGI330_HiTVGroup3Text;
    pVBInfo->Ren525pGroup3 = XGI330_Ren525pGroup3;
    pVBInfo->Ren750pGroup3 = XGI330_Ren750pGroup3;


    (void) memcpy(& pVBInfo->TimingH, XGI_TimingH, sizeof(XGI_TimingH));
    (void) memcpy(& pVBInfo->TimingV, XGI_TimingV, sizeof(XGI_TimingV));

    pVBInfo->CHTVVCLKUNTSC = XGI330_CHTVVCLKUNTSC;
    pVBInfo->CHTVVCLKONTSC = XGI330_CHTVVCLKONTSC;
    pVBInfo->CHTVVCLKUPAL = XGI330_CHTVVCLKUPAL;
    pVBInfo->CHTVVCLKOPAL = XGI330_CHTVVCLKOPAL;

    /* 310 customization related */
    if ((pVBInfo->VBType & VB_XGI301LV) || (pVBInfo->VBType & VB_XGI302LV))
        pVBInfo->LCDCapList = XGI_LCDDLCapList;
    else
        pVBInfo->LCDCapList = XGI_LCDCapList;

    pVBInfo->XGI_TVDelayList = XGI301TVDelayList;
    pVBInfo->XGI_TVDelayList2 = XGI301TVDelayList2;


    pVBInfo->I2CDefinition = XG40_I2CDefinition;

    if (ChipType == XG20)
        pVBInfo->CR97 = XG20_CR97;
}






/* --------------------------------------------------------------------- */
/* Function : XGISetModeNew */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
BOOLEAN
XGISetModeNew(PXGI_HW_DEVICE_INFO HwDeviceExtension, PVB_DEVICE_INFO pVBInfo,
	      USHORT ModeNo)
{
#ifndef LINUX_XF86
    ULONG temp;
    USHORT KeepLockReg;
#endif
    USHORT ModeIdIndex;
    /* PUCHAR pVBInfo->FBAddr = HwDeviceExtension->pjVideoMemoryAddress ; */
    USHORT temp_mode_no;

    pVBInfo->IF_DEF_VideoCapture = 1;
    pVBInfo->IF_DEF_ScaleLCD = 1;


    if (ModeNo & 0x80) {
        ModeNo = ModeNo & 0x7F;
    }

    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x05, 0x86);

    if (HwDeviceExtension->jChipType != XG20)   /* kuku 2004/06/25 1.Openkey */
        XGI_UnLockCRT2(HwDeviceExtension, pVBInfo);

    temp_mode_no = ModeNo;
    XGI_SearchModeID(pVBInfo->SModeIDTable, pVBInfo->EModeIDTable, 0x11,
                     &temp_mode_no, &ModeIdIndex);

    if (HwDeviceExtension->jChipType != XG20) { /* kuku 2004/06/25 */
        PDEBUG(ErrorF("XGI_GetVBInfo \n"));
        XGI_GetVBInfo(ModeNo, ModeIdIndex, HwDeviceExtension, pVBInfo);
        PDEBUG(ErrorF("XGI_GetTVInfo \n"));
        XGI_GetTVInfo(ModeNo, ModeIdIndex, pVBInfo);
        PDEBUG(ErrorF("XGI_GetLCDInfo \n"));
        XGI_GetLCDInfo(ModeNo, ModeIdIndex, pVBInfo);
        PDEBUG(ErrorF("XGI_DisableBridge \n"));
        XGI_DisableBridge(HwDeviceExtension, pVBInfo);


        if (pVBInfo->VBInfo & (SetSimuScanMode | SetCRT2ToLCDA)) {
            XGI_SetCRT1Group(HwDeviceExtension, ModeNo, ModeIdIndex, pVBInfo);

            if (pVBInfo->VBInfo & SetCRT2ToLCDA) {
                XGI_SetLCDAGroup(ModeNo, ModeIdIndex, HwDeviceExtension,
                                 pVBInfo);
            }
        }
        else {
            if (!(pVBInfo->VBInfo & SwitchToCRT2)) {
                XGI_SetCRT1Group(HwDeviceExtension, ModeNo, ModeIdIndex,
                                 pVBInfo);
                if (pVBInfo->VBInfo & SetCRT2ToLCDA) {
                    XGI_SetLCDAGroup(ModeNo, ModeIdIndex, HwDeviceExtension,
                                     pVBInfo);
                }
            }
        }

        PDEBUG(ErrorF(" vb_setmode 474\n"));    // yilin 
        if (pVBInfo->VBInfo & (SetSimuScanMode | SwitchToCRT2)) {
            switch (HwDeviceExtension->ujVBChipID) {
            case VB_CHIP_301:
                PDEBUG(ErrorF(" vb_setmode 301\n"));    //yilin
                XGI_SetCRT2Group301(ModeNo, HwDeviceExtension, pVBInfo);        /*add for CRT2 */
                break;

            case VB_CHIP_302:
                XGI_SetCRT2Group301(ModeNo, HwDeviceExtension, pVBInfo);        /*add for CRT2 */
                break;

            default:
                break;
            }
        }
        ErrorF("492 Part2 0 = %x ",
               XGI_GetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x0));
        XGI_SetCRT2ModeRegs(ModeNo, HwDeviceExtension, pVBInfo);
        XGI_OEM310Setting(ModeNo, ModeIdIndex, pVBInfo);        /*0212 */
        XGI_EnableBridge(HwDeviceExtension, pVBInfo);
        ErrorF("497 Part2 0 = %x ",
               XGI_GetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x0));
    }                           /* !XG20 */
    else {
        if (ModeNo <= 0x13) {
            pVBInfo->ModeType =
                pVBInfo->SModeIDTable[ModeIdIndex].St_ModeFlag & ModeInfoFlag;
        }
        else {
            pVBInfo->ModeType =
                pVBInfo->EModeIDTable[ModeIdIndex].
                Ext_ModeFlag & ModeInfoFlag;
        }
        pVBInfo->SetFlag = 0;
        pVBInfo->VBInfo = DisableCRT2Display;
        XGI_DisplayOff(pVBInfo);
        XGI_SetCRT1Group(HwDeviceExtension, ModeNo, ModeIdIndex, pVBInfo);
        XGI_DisplayOn(pVBInfo);
    }

/*
    if ( ModeNo <= 0x13 )
    {
        modeflag = pVBInfo->SModeIDTable[ ModeIdIndex ].St_ModeFlag ;
    }
    else
    {
        modeflag = pVBInfo->EModeIDTable[ ModeIdIndex ].Ext_ModeFlag ;
    }
    pVBInfo->ModeType = modeflag&ModeInfoFlag ;
    pVBInfo->SetFlag = 0x00 ;
    pVBInfo->VBInfo = DisableCRT2Display ;
    temp = XGINew_CheckMemorySize(  HwDeviceExtension , ModeNo , ModeIdIndex, pVBInfo ) ;

    if ( temp == 0 )
        return( 0 ) ;

    XGI_DisplayOff( pVBInfo) ;
    XGI_SetCRT1Group( HwDeviceExtension , ModeNo , ModeIdIndex, pVBInfo ) ;
    XGI_DisplayOn( pVBInfo) ;
*/
    ErrorF("Part2 0 = %x ",
           XGI_GetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x0));
    XGI_UpdateModeInfo(HwDeviceExtension, pVBInfo);

    if (HwDeviceExtension->jChipType != XG20)   /* kuku 2004/06/25 */
        XGI_LockCRT2(HwDeviceExtension, pVBInfo);

    return (TRUE);
}


/* --------------------------------------------------------------------- */
/* Function : XGI_SetCRT1Group */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_SetCRT1Group(PXGI_HW_DEVICE_INFO HwDeviceExtension, USHORT ModeNo,
                 USHORT ModeIdIndex, PVB_DEVICE_INFO pVBInfo)
{
    const USHORT StandTableIndex = XGI_GetModePtr(pVBInfo->SModeIDTable,
                                                  pVBInfo->ModeType,
                                                  ModeNo, ModeIdIndex);
    USHORT RefreshRateTableIndex;
    USHORT b3CC;
    USHORT temp;

    USHORT XGINew_P3cc = pVBInfo->P3cc;
#ifndef LINUX_XF86
    USHORT XGINew_P3c2 = pVBInfo->P3c2;
#endif

    /* XGINew_CRT1Mode = ModeNo ; // SaveModeID */
    XGI_SetSeqRegs(StandTableIndex, pVBInfo);
    XGI_SetMiscRegs(StandTableIndex, pVBInfo);
    XGI_SetCRTCRegs(StandTableIndex, pVBInfo);
    XGI_SetATTRegs(ModeNo, StandTableIndex, ModeIdIndex, pVBInfo);
    XGI_SetGRCRegs(StandTableIndex, pVBInfo);
    XGI_ClearExt1Regs(ModeNo, pVBInfo);

    temp = ~ProgrammingCRT2;
    pVBInfo->SetFlag &= temp;
    pVBInfo->SelectCRT2Rate = 0;

    if (pVBInfo->
        VBType & (VB_XGI301B | VB_XGI302B | VB_XGI301LV | VB_XGI302LV |
                  VB_XGI301C)) {
        if (pVBInfo->
            VBInfo & (SetSimuScanMode | SetCRT2ToLCDA | SetInSlaveMode)) {
            pVBInfo->SetFlag |= ProgrammingCRT2;
        }
    }

    RefreshRateTableIndex = XGI_GetRatePtrCRT2(ModeNo, ModeIdIndex, pVBInfo);

    if (RefreshRateTableIndex != 0xFFFF) {
        XGI_SetSync(RefreshRateTableIndex, pVBInfo);
        XGI_SetCRT1CRTC(ModeNo, ModeIdIndex, RefreshRateTableIndex, pVBInfo,
                        HwDeviceExtension);
        XGI_SetCRT1DE(HwDeviceExtension, ModeNo, ModeIdIndex,
                      RefreshRateTableIndex, pVBInfo);
        XGI_SetCRT1Offset(ModeNo, ModeIdIndex, RefreshRateTableIndex,
                          HwDeviceExtension, pVBInfo);
        XGI_SetCRT1VCLK(ModeNo, ModeIdIndex, HwDeviceExtension,
                        RefreshRateTableIndex, pVBInfo);
    }

    if (HwDeviceExtension->jChipType == XG20) {
        if ((ModeNo == 0x00) | (ModeNo == 0x01)) {
            XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x2B, 0x4E);
            XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x2C, 0xE9);
            b3CC = (UCHAR) XGI_GetRegByte((XGIIOADDRESS) XGINew_P3cc);
            XGI_SetRegByte((XGIIOADDRESS) XGINew_P3cc, (b3CC |= 0x0C));
        }
        else if ((ModeNo == 0x04) | (ModeNo == 0x05) | (ModeNo == 0x0D)) {
            XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x2B, 0x1B);
            XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x2C, 0xE3);
            b3CC = (UCHAR) XGI_GetRegByte((XGIIOADDRESS) XGINew_P3cc);
            XGI_SetRegByte((XGIIOADDRESS) XGINew_P3cc, (b3CC |= 0x0C));
        }
    }

    pVBInfo->SetFlag &= (~ProgrammingCRT2);
    XGI_SetCRT1FIFO(ModeNo, HwDeviceExtension, pVBInfo);
    XGI_SetCRT1ModeRegs(HwDeviceExtension, ModeNo, ModeIdIndex,
                        RefreshRateTableIndex, pVBInfo);

    if (HwDeviceExtension->jChipType == XG40) { /* Copy reg settings to 2nd chip */
        if (CheckDualChip(pVBInfo))
            SetDualChipRegs(HwDeviceExtension, pVBInfo);
    }

    /* XGI_LoadCharacter(); //dif ifdef TVFont */

    XGI_LoadDAC(ModeNo, ModeIdIndex, pVBInfo);
}


/* --------------------------------------------------------------------- */
/* Function : XGI_SetSeqRegs */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_SetSeqRegs(USHORT StandTableIndex, const VB_DEVICE_INFO *pVBInfo)
{
    unsigned SRdata;
    unsigned i;

    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x00, 0x03);       /* Set SR0 */
    SRdata = pVBInfo->StandTable[StandTableIndex].SR[0];

    if (pVBInfo->VBInfo & SetCRT2ToLCDA) {
        SRdata |= 0x01;
    }
    else {
        if (pVBInfo->VBInfo & (SetCRT2ToTV | SetCRT2ToLCD)) {
            if (pVBInfo->VBInfo & SetInSlaveMode)
                SRdata |= 0x01;
        }
    }

    SRdata |= 0x20;             /* screen off */
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x01, SRdata);     /* Set SR1 */

    /* Get SR2, SR3, and SR4 from table and set in hardware.
     */
    for (i = 2; i <= 4; i++) {
        SRdata = pVBInfo->StandTable[StandTableIndex].SR[i - 1];
        XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4, i, SRdata);
    }
}


/* --------------------------------------------------------------------- */
/* Function : XGI_SetMiscRegs */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_SetMiscRegs(USHORT StandTableIndex, const VB_DEVICE_INFO *pVBInfo)
{
    UCHAR Miscdata;

    Miscdata = pVBInfo->StandTable[StandTableIndex].MISC;       /* Get Misc from file */
/*
    if ( pVBInfo->VBType & ( VB_XGI301B | VB_XGI302B | VB_XGI301LV | VB_XGI302LV | VB_XGI301C ) )
    {
        if ( pVBInfo->VBInfo & SetCRT2ToLCDA )
        {
            Miscdata |= 0x0C ;
        }
    }
*/

    XGI_SetRegByte((XGIIOADDRESS) pVBInfo->P3c2, Miscdata);     /* Set Misc(3c2) */
}


/* --------------------------------------------------------------------- */
/* Function : XGI_SetCRTCRegs */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_SetCRTCRegs(unsigned StandTableIndex, const VB_DEVICE_INFO *pVBInfo)
{
    unsigned i;

    /* Unlock CRTC */
    XGI_SetRegAND((XGIIOADDRESS) pVBInfo->P3d4, 0x11, 0x7f);

    for (i = 0; i <= 0x18; i++) {
        /* Get CRTC from file */
        const unsigned CRTCdata = 
            pVBInfo->StandTable[StandTableIndex].CRTC[i];

        /* Set CRTC( 3d4 ) */
        XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4, i, CRTCdata);
    }
}


/* --------------------------------------------------------------------- */
/* Function : */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_SetATTRegs(unsigned ModeNo, unsigned StandTableIndex, unsigned ModeIdIndex,
               const VB_DEVICE_INFO *pVBInfo)
{
    unsigned i;
    const unsigned modeflag = (ModeNo <= 0x13)
        ? pVBInfo->SModeIDTable[ModeIdIndex].St_ModeFlag
        : pVBInfo->EModeIDTable[ModeIdIndex].Ext_ModeFlag;

    for (i = 0; i <= 0x13; i++) {
        UCHAR ARdata = pVBInfo->StandTable[StandTableIndex].ATTR[i];

        if (modeflag & Charx8Dot) {     /* ifndef Dot9 */
            if (i == 0x13) {
                /* Pixel shift. If screen on LCD or TV is shifted left or
                 * right, this might be the cause.
                 */
                if (pVBInfo->VBInfo & SetCRT2ToLCDA)
                    ARdata = 0;
                else {
                    if (pVBInfo->VBInfo & (SetCRT2ToTV | SetCRT2ToLCD)) {
                        if (pVBInfo->VBInfo & SetInSlaveMode)
                            ARdata = 0;
                    }
                }
            }
        }

        XGI_GetRegByte((XGIIOADDRESS) pVBInfo->P3da);   /* reset 3da */
        XGI_SetRegByte((XGIIOADDRESS) pVBInfo->P3c0, i);        /* set index */
        XGI_SetRegByte((XGIIOADDRESS) pVBInfo->P3c0, ARdata);   /* set data */
    }

    XGI_GetRegByte((XGIIOADDRESS) pVBInfo->P3da);       /* reset 3da */
    XGI_SetRegByte((XGIIOADDRESS) pVBInfo->P3c0, 0x14); /* set index */
    XGI_SetRegByte((XGIIOADDRESS) pVBInfo->P3c0, 0x00); /* set data */

    XGI_GetRegByte((XGIIOADDRESS) pVBInfo->P3da);       /* Enable Attribute */
    XGI_SetRegByte((XGIIOADDRESS) pVBInfo->P3c0, 0x20);
    XGI_GetRegByte((XGIIOADDRESS) pVBInfo->P3da);
}


/* --------------------------------------------------------------------- */
/* Function : XGI_SetGRCRegs */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_SetGRCRegs(unsigned StandTableIndex, const VB_DEVICE_INFO *pVBInfo)
{
    unsigned i;

    for (i = 0; i <= 8; i++) {
        /* Get GR from file and set GR (3ce)
         */
        const unsigned GRdata = pVBInfo->StandTable[StandTableIndex].GRC[i];
        XGI_SetReg((XGIIOADDRESS) pVBInfo->P3ce, i, GRdata);
    }

    if (pVBInfo->ModeType > ModeVGA) {
        /* 256 color disable */
        XGI_SetRegAND((XGIIOADDRESS) pVBInfo->P3ce, 0x05, 0xBF);
    }
}


/* --------------------------------------------------------------------- */
/* Function : XGI_ClearExt1Regs */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_ClearExt1Regs(unsigned ModeNo, const VB_DEVICE_INFO *pVBInfo)
{
    unsigned i;

    /* Clear SR0A-SR0E */
    for (i = 0x0A; i <= 0x0E; i++) {
        XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4, i, 0x00);
    }

    /* This code came from the old XGI_New_ClearExt1Regs in init.c.  Since
     * it wasn't included in the newer code drop from XGI, I'm not sure if
     * it's necessary on the Volari chips.  I've included it here, ifdefed
     * out, for future reference.
     * - idr
     */
#if 0
    XGI_SetRegAND(pVBInfo->P3c4, 0x37, 0xFE);
    if ((ModeNo == 0x06) || ((ModeNo >= 0x0e) && (ModeNo <= 0x13))) {
        XGI_SetReg(pVBInfo->P3c4, 0x0e, 0x20);
    }
#else
    (void) ModeNo;
#endif
}


/* --------------------------------------------------------------------- */
/* Function : XGI_GetRatePtrCRT2 */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
USHORT
XGI_GetRatePtrCRT2(USHORT ModeNo, USHORT ModeIdIndex, PVB_DEVICE_INFO pVBInfo)
{
    SHORT LCDRefreshIndex[] = { 0x00, 0x00, 0x03, 0x01 }
    , LCDARefreshIndex[] = {
    0x00, 0x00, 0x03, 0x01, 0x01, 0x01, 0x01};

    USHORT RefreshRateTableIndex, i, modeflag, index, temp;

    if (ModeNo <= 0x13) {
        modeflag = pVBInfo->SModeIDTable[ModeIdIndex].St_ModeFlag;
    }
    else {
        modeflag = pVBInfo->EModeIDTable[ModeIdIndex].Ext_ModeFlag;
    }

    if (ModeNo < 0x14)
        return (0xFFFF);

    index = XGI_GetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x33);
    index = index >> pVBInfo->SelectCRT2Rate;
    index &= 0x0F;

    if (pVBInfo->LCDInfo & (LCDNonExpanding | EnableScalingLCD))
        index = 0;

    if (index > 0)
        index--;

    if (pVBInfo->SetFlag & ProgrammingCRT2) {
        if (pVBInfo->VBInfo & (SetCRT2ToLCD | SetCRT2ToLCDA)) {
            if (pVBInfo->
                VBType & (VB_XGI301B | VB_XGI302B | VB_XGI301LV | VB_XGI302LV
                          | VB_XGI301C))
                temp = LCDARefreshIndex[pVBInfo->LCDResInfo & 0x0F];    /* 301b */
            else
                temp = LCDRefreshIndex[pVBInfo->LCDResInfo & 0x0F];

            if (index > temp) {
                index = temp;
            }
        }
    }

    RefreshRateTableIndex = pVBInfo->EModeIDTable[ModeIdIndex].REFindex;
    ModeNo = pVBInfo->RefIndex[RefreshRateTableIndex].ModeID;
    i = 0;

    do {
        if (pVBInfo->RefIndex[RefreshRateTableIndex + i].ModeID != ModeNo)
            break;
        temp = pVBInfo->RefIndex[RefreshRateTableIndex + i].Ext_InfoFlag;
        temp &= ModeInfoFlag;
        if (temp < pVBInfo->ModeType)
            break;

        i++;
        index--;

    } while (index != 0xFFFF);

    if (!(pVBInfo->VBInfo & SetCRT2ToRAMDAC)) {
        if (pVBInfo->VBInfo & SetInSlaveMode) {
            temp =
                pVBInfo->RefIndex[RefreshRateTableIndex + i - 1].Ext_InfoFlag;
            if (temp & InterlaceMode) {
                i++;
            }
        }
    }

    i--;
    if ((pVBInfo->SetFlag & ProgrammingCRT2)) {
        temp =
            XGI_AjustCRT2Rate(ModeNo, ModeIdIndex, RefreshRateTableIndex, &i,
                              pVBInfo);
    }
    return (RefreshRateTableIndex + i); /*return(0x01|(temp1<<1));   */
}


/* --------------------------------------------------------------------- */
/* Function : XGI_AjustCRT2Rate */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
BOOLEAN
XGI_AjustCRT2Rate(USHORT ModeNo, USHORT ModeIdIndex,
                  USHORT RefreshRateTableIndex, USHORT * i,
                  PVB_DEVICE_INFO pVBInfo)
{
    USHORT tempax, tempbx, resinfo, modeflag, infoflag;

    if (ModeNo <= 0x13) {
        modeflag = pVBInfo->SModeIDTable[ModeIdIndex].St_ModeFlag;      /* si+St_ModeFlag */
    }
    else {
        modeflag = pVBInfo->EModeIDTable[ModeIdIndex].Ext_ModeFlag;
    }

    resinfo = pVBInfo->EModeIDTable[ModeIdIndex].Ext_RESINFO;
    tempbx = pVBInfo->RefIndex[RefreshRateTableIndex + (*i)].ModeID;
    tempax = 0;

    if (pVBInfo->VBInfo & SetCRT2ToRAMDAC) {
        tempax |= SupportRAMDAC2;

        if (pVBInfo->VBType & VB_XGI301C)
            tempax |= SupportCRT2in301C;
    }

    if (pVBInfo->VBInfo & (SetCRT2ToLCD | SetCRT2ToLCDA)) {     /* 301b */
        tempax |= SupportLCD;

        if (pVBInfo->LCDResInfo != Panel1280x1024) {
            if (pVBInfo->LCDResInfo != Panel1280x960) {
                if (pVBInfo->LCDInfo & LCDNonExpanding) {
                    if (resinfo >= 9) {
                        tempax = 0;
                        return (0);
                    }
                }
            }
        }
    }

    if (pVBInfo->VBInfo & SetCRT2ToHiVisionTV) {        /* for HiTV */
        if ((pVBInfo->VBType & VB_XGI301LV)
            && (pVBInfo->VBExtInfo == VB_YPbPr1080i)) {
            tempax |= SupportYPbPr;
            if (pVBInfo->VBInfo & SetInSlaveMode) {
                if (resinfo == 4)
                    return (0);

                if (resinfo == 3)
                    return (0);

                if (resinfo > 7)
                    return (0);
            }
        }
        else {
            tempax |= SupportHiVisionTV;
            if (pVBInfo->VBInfo & SetInSlaveMode) {
                if (resinfo == 4)
                    return (0);

                if (resinfo == 3) {
                    if (pVBInfo->SetFlag & TVSimuMode)
                        return (0);
                }

                if (resinfo > 7)
                    return (0);
            }
        }
    }
    else {
        if (pVBInfo->
            VBInfo & (SetCRT2ToAVIDEO | SetCRT2ToSVIDEO | SetCRT2ToSCART |
                      SetCRT2ToYPbPr | SetCRT2ToHiVisionTV)) {
            tempax |= SupportTV;

            if (pVBInfo->
                VBType & (VB_XGI301B | VB_XGI302B | VB_XGI301LV | VB_XGI302LV
                          | VB_XGI301C)) {
                tempax |= SupportTV1024;
            }

            if (!(pVBInfo->VBInfo & SetPALTV)) {
                if (modeflag & NoSupportSimuTV) {
                    if (pVBInfo->VBInfo & SetInSlaveMode) {
                        if (!(pVBInfo->VBInfo & SetNotSimuMode)) {
                            return (0);
                        }
                    }
                }
            }
        }
    }

    for (; pVBInfo->RefIndex[RefreshRateTableIndex + (*i)].ModeID == tempbx;
         (*i)--) {
        infoflag =
            pVBInfo->RefIndex[RefreshRateTableIndex + (*i)].Ext_InfoFlag;
        if (infoflag & tempax) {
            return (1);
        }
        if ((*i) == 0)
            break;
    }

    for ((*i) = 0;; (*i)++) {
        infoflag =
            pVBInfo->RefIndex[RefreshRateTableIndex + (*i)].Ext_InfoFlag;
        if (pVBInfo->RefIndex[RefreshRateTableIndex + (*i)].ModeID != tempbx) {
            return (0);
        }

        if (infoflag & tempax) {
            return (1);
        }
    }
    return (1);
}


/* --------------------------------------------------------------------- */
/* Function : XGI_SetSync */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_SetSync(unsigned RefreshRateTableIndex, const VB_DEVICE_INFO *pVBInfo)
{
    const unsigned sync =
	(pVBInfo->RefIndex[RefreshRateTableIndex].Ext_InfoFlag >> 8) & 0xC0;

    /* Set Misc(3c2) */
    XGI_SetRegByte((XGIIOADDRESS) pVBInfo->P3c2, sync | 0x2F);
}


/* --------------------------------------------------------------------- */
/* Function : XGI_SetCRT1CRTC */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_SetCRT1CRTC(USHORT ModeNo, USHORT ModeIdIndex,
                USHORT RefreshRateTableIndex, PVB_DEVICE_INFO pVBInfo,
                PXGI_HW_DEVICE_INFO HwDeviceExtension)
{
    UCHAR index, data;
#ifndef LINUX_XF86
    USHORT temp, tempah, j, modeflag, ResInfo, DisplayType;
#endif
    USHORT i;

    index = pVBInfo->RefIndex[RefreshRateTableIndex].Ext_CRT1CRTC;      /* Get index */
    index = index & IndexMask;

    data = (UCHAR) XGI_GetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x11);
    data &= 0x7F;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x11, data);       /* Unlock CRTC */

    for (i = 0; i < 8; i++)
        pVBInfo->TimingH.data[i] =
            pVBInfo->XGINEWUB_CRT1Table[index].CR[i];

    for (i = 0; i < 7; i++)
        pVBInfo->TimingV.data[i] =
            pVBInfo->XGINEWUB_CRT1Table[index].CR[i + 8];

    XGI_SetCRT1Timing_H(pVBInfo, HwDeviceExtension);



    XGI_SetCRT1Timing_V(ModeIdIndex, ModeNo, pVBInfo);


    if (pVBInfo->ModeType > 0x03)
        XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x14, 0x4F);
}


/* --------------------------------------------------------------------- */
/* Function : XGI_SetCRT1Timing_H */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_SetCRT1Timing_H(PVB_DEVICE_INFO pVBInfo,
                    PXGI_HW_DEVICE_INFO HwDeviceExtension)
{
    UCHAR data, data1, pushax;
    USHORT i, j;

    /* XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4 , 0x51 , 0 ) ; */
    /* XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4 , 0x56 , 0 ) ; */
    /* XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->P3d4 ,0x11 , 0x7f , 0x00 ) ; */

    data = (UCHAR) XGI_GetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x11);      /* unlock cr0-7 */
    data &= 0x7F;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x11, data);

    data = pVBInfo->TimingH.data[0];
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4, 0, data);

    for (i = 0x01; i <= 0x04; i++) {
        data = pVBInfo->TimingH.data[i];
        XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4, (USHORT) (i + 1), data);
    }

    for (i = 0x05; i <= 0x06; i++) {
        data = pVBInfo->TimingH.data[i];
        XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4, (USHORT) (i + 6), data);
    }

    j = (UCHAR) XGI_GetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x0e);
    j &= 0x1F;
    data = pVBInfo->TimingH.data[7];
    data &= 0xE0;
    data |= j;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x0e, data);

    if (HwDeviceExtension->jChipType == XG20) {
        data = (UCHAR) XGI_GetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x04);
        data = data - 1;
        XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x04, data);
        data = (UCHAR) XGI_GetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x05);
        data1 = data;
        data1 &= 0xE0;
        data &= 0x1F;
        if (data == 0) {
            pushax = data;
            data = (UCHAR) XGI_GetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x0c);
            data &= 0xFB;
            XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x0c, data);
            data = pushax;
        }
        data = data - 1;
        data |= data1;
        XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x05, data);
        data = (UCHAR) XGI_GetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x0e);
        data = data >> 5;
        data = data + 3;
        if (data > 7)
            data = data - 7;
        data = data << 5;
        XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->P3c4, 0x0e, ~0xE0, data);
    }
}


/* --------------------------------------------------------------------- */
/* Function : XGI_SetCRT1Timing_V */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_SetCRT1Timing_V(USHORT ModeIdIndex, USHORT ModeNo,
                    PVB_DEVICE_INFO pVBInfo)
{
    UCHAR data;
    USHORT i, j;

    /* XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4 , 0x51 , 0 ) ; */
    /* XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4 , 0x56 , 0 ) ; */
    /* XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->P3d4 , 0x11 , 0x7f , 0x00 ) ; */

    for (i = 0x00; i <= 0x01; i++) {
        data = pVBInfo->TimingV.data[i];
        XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4, (USHORT) (i + 6), data);
    }

    for (i = 0x02; i <= 0x03; i++) {
        data = pVBInfo->TimingV.data[i];
        XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4, (USHORT) (i + 0x0e), data);
    }

    for (i = 0x04; i <= 0x05; i++) {
        data = pVBInfo->TimingV.data[i];
        XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4, (USHORT) (i + 0x11), data);
    }

    j = (UCHAR) XGI_GetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x0a);
    j &= 0xC0;
    data = pVBInfo->TimingV.data[6];
    data &= 0x3F;
    data |= j;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x0a, data);

    data = pVBInfo->TimingV.data[6];
    data &= 0x80;
    data = data >> 2;

    if (ModeNo <= 0x13)
        i = pVBInfo->SModeIDTable[ModeIdIndex].St_ModeFlag;
    else
        i = pVBInfo->EModeIDTable[ModeIdIndex].Ext_ModeFlag;

    i &= DoubleScanMode;
    if (i)
        data |= 0x80;

    j = (UCHAR) XGI_GetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x09);
    j &= 0x5F;
    data |= j;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x09, data);
}


/* --------------------------------------------------------------------- */
/* Function : XGI_SetCRT1DE */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_SetCRT1DE(PXGI_HW_DEVICE_INFO HwDeviceExtension, USHORT ModeNo,
              USHORT ModeIdIndex, USHORT RefreshRateTableIndex,
              PVB_DEVICE_INFO pVBInfo)
{
    USHORT tempax, tempbx, tempcx, temp, modeflag;
    UCHAR data;
    const USHORT resindex = XGI_GetResInfo(ModeNo, ModeIdIndex, pVBInfo);


    if (ModeNo <= 0x13) {
        modeflag = pVBInfo->SModeIDTable[ModeIdIndex].St_ModeFlag;
        tempax = pVBInfo->StResInfo[resindex].HTotal;
        tempbx = pVBInfo->StResInfo[resindex].VTotal;
    }
    else {
        modeflag = pVBInfo->EModeIDTable[ModeIdIndex].Ext_ModeFlag;
        tempax = pVBInfo->ModeResInfo[resindex].HTotal;
        tempbx = pVBInfo->ModeResInfo[resindex].VTotal;
    }

    if (modeflag & HalfDCLK)
        tempax = tempax >> 1;

    if (ModeNo > 0x13) {
        if (modeflag & HalfDCLK)
            tempax = tempax << 1;

        temp = pVBInfo->RefIndex[RefreshRateTableIndex].Ext_InfoFlag;

        if (temp & InterlaceMode)
            tempbx = tempbx >> 1;

        if (modeflag & DoubleScanMode)
            tempbx = tempbx << 1;
    }

    tempcx = 8;

    /* if ( !( modeflag & Charx8Dot ) ) */
    /* tempcx = 9 ; */

    tempax /= tempcx;
    tempax -= 1;
    tempbx -= 1;
    tempcx = tempax;
    temp = (UCHAR) XGI_GetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x11);
    data = (UCHAR) XGI_GetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x11);
    data &= 0x7F;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x11, data);       /* Unlock CRTC */
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x01, (USHORT) (tempcx & 0xff));
    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->P3d4, 0x0b, ~0x0c,
                    (USHORT) ((tempcx & 0x0ff00) >> 10));
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x12, (USHORT) (tempbx & 0xff));
    tempax = 0;
    tempbx = tempbx >> 8;

    if (tempbx & 0x01)
        tempax |= 0x02;

    if (tempbx & 0x02)
        tempax |= 0x40;

    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->P3d4, 0x07, ~0x42, tempax);
    data = (UCHAR) XGI_GetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x07);
    data &= 0xFF;
    tempax = 0;

    if (tempbx & 0x04)
        tempax |= 0x02;

    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->P3d4, 0x0a, ~0x02, tempax);
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x11, temp);
}


/* --------------------------------------------------------------------- */
/* Function : XGI_GetResInfo */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
USHORT
XGI_GetResInfo(USHORT ModeNo, USHORT ModeIdIndex, PVB_DEVICE_INFO pVBInfo)
{
    return (ModeNo <= 0x13)
        ? pVBInfo->SModeIDTable[ModeIdIndex].St_ResInfo
        : pVBInfo->EModeIDTable[ModeIdIndex].Ext_RESINFO;
}


static void
get_mode_xres_yres(USHORT ModeNo, USHORT ModeIdIndex, PVB_DEVICE_INFO pVBInfo,
                   unsigned *width, unsigned *height)
{
    const USHORT resindex = XGI_GetResInfo(ModeNo, ModeIdIndex, pVBInfo);
    unsigned xres;
    unsigned yres;


    if (ModeNo <= 0x13) {
        xres = pVBInfo->StResInfo[resindex].HTotal;
        yres = pVBInfo->StResInfo[resindex].VTotal;
    }
    else {
        const unsigned modeflag =
            pVBInfo->EModeIDTable[ModeIdIndex].Ext_ModeFlag;

        xres = pVBInfo->ModeResInfo[resindex].HTotal;
        yres = pVBInfo->ModeResInfo[resindex].VTotal;

        if (modeflag & HalfDCLK)
            xres *= 2;

        if (modeflag & DoubleScanMode)
            yres *= 2;
    }

    *width = xres;
    *height = yres;
}


/* --------------------------------------------------------------------- */
/* Function : XGI_SetCRT1Offset */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_SetCRT1Offset(USHORT ModeNo, USHORT ModeIdIndex,
                  USHORT RefreshRateTableIndex,
                  PXGI_HW_DEVICE_INFO HwDeviceExtension,
                  PVB_DEVICE_INFO pVBInfo)
{
    USHORT temp, ah, al, temp2, i, DisplayUnit;

    /* GetOffset */
    temp = pVBInfo->EModeIDTable[ModeIdIndex].Ext_ModeInfo;
    temp = temp >> 8;
    temp = pVBInfo->ScreenOffset[temp];

    temp2 = pVBInfo->RefIndex[RefreshRateTableIndex].Ext_InfoFlag;
    temp2 &= InterlaceMode;

    if (temp2)
        temp = temp << 1;

    temp2 = pVBInfo->ModeType - ModeEGA;

    switch (temp2) {
    case 0:
        temp2 = 1;
        break;
    case 1:
        temp2 = 2;
        break;
    case 2:
        temp2 = 4;
        break;
    case 3:
        temp2 = 4;
        break;
    case 4:
        temp2 = 6;
        break;
    case 5:
        temp2 = 8;
        break;
    default:
        break;
    }

    if ((ModeNo >= 0x26) && (ModeNo <= 0x28))
        temp = temp * temp2 + temp2 / 2;
    else
        temp *= temp2;

    /* SetOffset */
    DisplayUnit = temp;
    temp2 = temp;
    temp = temp >> 8;           /* ah */
    temp &= 0x0F;
    i = XGI_GetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x0E);
    i &= 0xF0;
    i |= temp;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x0E, i);

    temp = (UCHAR) temp2;
    temp &= 0xFF;               /* al */
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x13, temp);

    /* SetDisplayUnit */
    temp2 = pVBInfo->RefIndex[RefreshRateTableIndex].Ext_InfoFlag;
    temp2 &= InterlaceMode;
    if (temp2)
        DisplayUnit >>= 1;

    DisplayUnit = DisplayUnit << 5;
    ah = (DisplayUnit & 0xff00) >> 8;
    al = DisplayUnit & 0x00ff;
    if (al == 0)
        ah += 1;
    else
        ah += 2;

    if (HwDeviceExtension->jChipType == XG20)
        if ((ModeNo == 0x4A) | (ModeNo == 0x49))
            ah -= 1;

    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x10, ah);
}


/* --------------------------------------------------------------------- */
/* Function : XGI_SetCRT1VCLK */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_SetCRT1VCLK(USHORT ModeNo, USHORT ModeIdIndex,
                PXGI_HW_DEVICE_INFO HwDeviceExtension,
                USHORT RefreshRateTableIndex, PVB_DEVICE_INFO pVBInfo)
{
    unsigned index;
    unsigned clka;
    unsigned clkb;

    if ((pVBInfo->VBType & VB_XGI301BLV302BLV)
        && (pVBInfo->VBInfo & SetCRT2ToLCDA)) {
        index = XGI_GetVCLK2Ptr(ModeNo, ModeIdIndex, RefreshRateTableIndex,
                                pVBInfo);

        clka = pVBInfo->VBVCLKData[index].Part4_A;
        clkb = pVBInfo->VBVCLKData[index].Part4_B;
    }
    else {
        index = pVBInfo->RefIndex[RefreshRateTableIndex].Ext_CRTVCLK;

        clka = pVBInfo->VCLKData[index].SR2B;
        clkb = pVBInfo->VCLKData[index].SR2C;
    }

    XGI_SetRegAND((XGIIOADDRESS) pVBInfo->P3c4, 0x31, 0xCF);
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x2B, clka);
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x2C, clkb);
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x2D, 0x01);

    if ((HwDeviceExtension->jChipType == XG20) 
         && (pVBInfo->EModeIDTable[ModeIdIndex].Ext_ModeFlag & HalfDCLK)) {
        UCHAR data;

        /* FIXME: Does this actually serve any purpose?  This register is
         * FIXME: already written above.
         */
        data = XGI_GetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x2B);
        XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x2B, data);

        /* FIXME: The logic here seems wrong.  It looks like its possible
         * FIXME: for the (data << 1) to cause a bit to creep into the index
         * FIXME: part.  THere's no documentation for this register, so I have
         * FIXME: no way of knowing. :(
         */
        data = XGI_GetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x2C);
        index = data;
        index &= 0xE0;
        data &= 0x1F;
        data = data << 1;
        data += 1;
        data |= index;
        XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x2C, data);
    }
}


/* --------------------------------------------------------------------- */
/* Function : XGI_SetCRT1FIFO */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_SetCRT1FIFO(USHORT ModeNo, PXGI_HW_DEVICE_INFO HwDeviceExtension,
                PVB_DEVICE_INFO pVBInfo)
{
    USHORT data;

    data = XGI_GetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x3D);
    data &= 0xfe;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x3D, data);       /* diable auto-threshold */

    if (ModeNo > 0x13) {
        XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x08, 0x34);
        data = XGI_GetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x09);
        data &= 0xF0;
        XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x09, data);
        data = XGI_GetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x3D);
        data |= 0x01;
        XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x3D, data);
    }
    else {
        XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x08, 0xAE);
        data = XGI_GetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x09);
        data &= 0xF0;
        XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x09, data);
    }
}


/* --------------------------------------------------------------------- */
/* Function : XGI_SetCRT1ModeRegs */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_SetCRT1ModeRegs(PXGI_HW_DEVICE_INFO HwDeviceExtension,
                    USHORT ModeNo, USHORT ModeIdIndex,
                    USHORT RefreshRateTableIndex, PVB_DEVICE_INFO pVBInfo)
{
    USHORT data, data2, data3, infoflag = 0, modeflag, resindex, xres;

    if (ModeNo > 0x13) {
        modeflag = pVBInfo->EModeIDTable[ModeIdIndex].Ext_ModeFlag;
        infoflag = pVBInfo->RefIndex[RefreshRateTableIndex].Ext_InfoFlag;
    }
    else
        modeflag = pVBInfo->SModeIDTable[ModeIdIndex].St_ModeFlag;      /* si+St_ModeFlag */

    if (XGI_GetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x31) & 0x01)
        XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->P3c4, 0x1F, 0x3F, 0x00);

    if (ModeNo > 0x13)
        data = infoflag;
    else
        data = 0;

    data2 = 0;

    if (ModeNo > 0x13) {
        if (pVBInfo->ModeType > 0x02) {
            data2 |= 0x02;
            data3 = pVBInfo->ModeType - ModeVGA;
            data3 = data3 << 2;
            data2 |= data3;
        }
    }

    data &= InterlaceMode;

    if (data)
        data2 |= 0x20;

    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->P3c4, 0x06, ~0x3F, data2);
    /* XGI_SetReg((XGIIOADDRESS)pVBInfo->P3c4,0x06,data2); */
    resindex = XGI_GetResInfo(ModeNo, ModeIdIndex, pVBInfo);
    if (ModeNo <= 0x13)
        xres = pVBInfo->StResInfo[resindex].HTotal;
    else
        xres = pVBInfo->ModeResInfo[resindex].HTotal;   /* xres->ax */

    data = 0x0000;
    if (infoflag & InterlaceMode) {
        if (xres == 1024)
            data = 0x0035;
        else if (xres == 1280)
            data = 0x0048;
    }

    data2 = data & 0x00FF;
    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->P3d4, 0x19, 0xFF, data2);
    data2 = (data & 0xFF00) >> 8;
    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->P3d4, 0x19, 0xFC, data2);

    if (modeflag & HalfDCLK)
        XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->P3c4, 0x01, 0xF7, 0x08);

    data2 = 0;

    if (modeflag & LineCompareOff)
        data2 |= 0x08;

    if (ModeNo > 0x13) {
        if (pVBInfo->ModeType == ModeEGA)
            data2 |= 0x40;
    }

    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->P3c4, 0x0F, ~0x48, data2);
    data = 0x60;
    if (pVBInfo->ModeType != ModeText) {
        data = data ^ 0x60;
        if (pVBInfo->ModeType != ModeEGA) {
            data = data ^ 0xA0;
        }
    }
    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->P3c4, 0x21, 0x1F, data);

    XGI_SetVCLKState(HwDeviceExtension, ModeNo, RefreshRateTableIndex,
                     pVBInfo);

    data = XGI_GetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x31);

    if (HwDeviceExtension->jChipType == XG20) {
        if (data & 0x40)
            data = 0x33;
        else
            data = 0x73;
        XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x52, data);
        XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x51, 0x02);
    }
    else {
        if (data & 0x40)
            data = 0x2c;
        else
            data = 0x6c;
        XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x52, data);
    }

}


/* --------------------------------------------------------------------- */
/* Function : XGI_SetVCLKState */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_SetVCLKState(PXGI_HW_DEVICE_INFO HwDeviceExtension, USHORT ModeNo,
                 USHORT RefreshRateTableIndex, PVB_DEVICE_INFO pVBInfo)
{
    USHORT data, data2 = 0;
    SHORT VCLK;

    UCHAR index;

    if (ModeNo <= 0x13)
        VCLK = 0;
    else {
        index = pVBInfo->RefIndex[RefreshRateTableIndex].Ext_CRTVCLK;
        index &= IndexMask;
        VCLK = pVBInfo->VCLKData[index].CLOCK;
    }

    data = XGI_GetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x32);
    data &= 0xf3;
    if (VCLK >= 200)
        data |= 0x0c;           /* VCLK > 200 */

    if (HwDeviceExtension->jChipType == XG20)
        data &= ~0x04;          /* 2 pixel mode */

    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x32, data);

    if (HwDeviceExtension->jChipType != XG20) {
        data = XGI_GetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x1F);
        data &= 0xE7;
        if (VCLK < 200)
            data |= 0x10;
        XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x1F, data);
    }

    if ((VCLK >= 0) && (VCLK < 135))
        data2 = 0x03;
    else if ((VCLK >= 135) && (VCLK < 160))
        data2 = 0x02;
    else if ((VCLK >= 160) && (VCLK < 260))
        data2 = 0x01;
    else if (VCLK > 260)
        data2 = 0x00;

    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->P3c4, 0x07, 0xFC, data2);
}


/* --------------------------------------------------------------------- */
/* Function : XGI_LoadDAC */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_LoadDAC(USHORT ModeNo, USHORT ModeIdIndex, PVB_DEVICE_INFO pVBInfo)
{
    USHORT data, data2, time, i, j, k, m, n, o, si, di, bx, dl, al, ah, dh;
    const uint8_t *table = NULL;

    if (ModeNo <= 0x13)
        data = pVBInfo->SModeIDTable[ModeIdIndex].St_ModeFlag;
    else
        data = pVBInfo->EModeIDTable[ModeIdIndex].Ext_ModeFlag;

    data &= DACInfoFlag;
    time = 64;

    if (data == 0x00)
        table = XGI_MDA_DAC;
    else if (data == 0x08)
        table = XGI_CGA_DAC;
    else if (data == 0x10)
        table = XGI_EGA_DAC;
    else if (data == 0x18) {
        time = 256;
        table = XGI_VGA_DAC;
    }

    if (time == 256)
        j = 16;
    else
        j = time;

    XGI_SetRegByte((XGIIOADDRESS) pVBInfo->P3c6, 0xFF);
    XGI_SetRegByte((XGIIOADDRESS) pVBInfo->P3c8, 0x00);

    for (i = 0; i < j; i++) {
        data = table[i];

        for (k = 0; k < 3; k++) {
            data2 = 0;

            if (data & 0x01)
                data2 = 0x2A;

            if (data & 0x02)
                data2 += 0x15;

            XGI_SetRegByte((XGIIOADDRESS) pVBInfo->P3c9, data2);
            data = data >> 2;
        }
    }

    if (time == 256) {
        for (i = 16; i < 32; i++) {
            data = table[i];

            for (k = 0; k < 3; k++)
                XGI_SetRegByte((XGIIOADDRESS) pVBInfo->P3c9, data);
        }

        si = 32;

        for (m = 0; m < 9; m++) {
            di = si;
            bx = si + 0x04;
            dl = 0;

            for (n = 0; n < 3; n++) {
                for (o = 0; o < 5; o++) {
                    dh = table[si];
                    ah = table[di];
                    al = table[bx];
                    si++;
                    XGI_WriteDAC((XGIIOADDRESS) pVBInfo->P3c9, 0, dl, 
				 ah, al, dh);
                }

                si -= 2;

                for (o = 0; o < 3; o++) {
                    dh = table[bx];
                    ah = table[di];
                    al = table[si];
                    si--;
                    XGI_WriteDAC((XGIIOADDRESS) pVBInfo->P3c9, 0, dl, 
				 ah, al, dh);
                }

                dl++;
            }

            si += 5;
        }
    }
}


/* --------------------------------------------------------------------- */
/* Function : XGI_WriteDAC */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_WriteDAC(XGIIOADDRESS dac_data, unsigned shift, unsigned ordering,
	     uint8_t ah, uint8_t al, uint8_t dh)
{
    USHORT temp, bh, bl;

    if (shift) {
	ah <<= 2;
	al <<= 2;
	dh <<= 2;
    }

    bh = ah;
    bl = al;

    if (ordering != 0) {
        temp = bh;
        bh = dh;
        dh = temp;
        if (ordering == 1) {
            temp = bl;
            bl = dh;
            dh = temp;
        }
        else {
            temp = bl;
            bl = bh;
            bh = temp;
        }
    }
    XGI_SetRegByte(dac_data, dh);
    XGI_SetRegByte(dac_data, bh);
    XGI_SetRegByte(dac_data, bl);
}


/* --------------------------------------------------------------------- */
/* Function : XGI_SetLCDAGroup */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_SetLCDAGroup(USHORT ModeNo, USHORT ModeIdIndex,
                 PXGI_HW_DEVICE_INFO HwDeviceExtension,
                 PVB_DEVICE_INFO pVBInfo)
{
    USHORT RefreshRateTableIndex;
    /* USHORT temp ; */

    /* pVBInfo->SelectCRT2Rate = 0 ; */

    pVBInfo->SetFlag |= ProgrammingCRT2;
    RefreshRateTableIndex = XGI_GetRatePtrCRT2(ModeNo, ModeIdIndex, pVBInfo);
    XGI_GetLVDSResInfo(ModeNo, ModeIdIndex, pVBInfo);
    XGI_GetLVDSData(ModeNo, ModeIdIndex, RefreshRateTableIndex, pVBInfo);
    XGI_ModCRT1Regs(ModeNo, ModeIdIndex, RefreshRateTableIndex,
                    HwDeviceExtension, pVBInfo);
    XGI_SetLVDSRegs(ModeNo, ModeIdIndex, RefreshRateTableIndex, pVBInfo);
    XGI_SetCRT2ECLK(ModeNo, ModeIdIndex, RefreshRateTableIndex, pVBInfo);
}


/**
 * Get LVDS resolution information.
 */
void
XGI_GetLVDSResInfo(USHORT ModeNo, USHORT ModeIdIndex, PVB_DEVICE_INFO pVBInfo)
{
    unsigned xres;
    unsigned yres;


    get_mode_xres_yres(ModeNo, ModeIdIndex, pVBInfo, &xres, &yres);

    if (xres == 720)
        xres = 640;

    pVBInfo->VGAHDE = xres;
    pVBInfo->HDE = xres;
    pVBInfo->VGAVDE = yres;
    pVBInfo->VDE = yres;
}


static void
get_HDE_VDE(PVB_DEVICE_INFO pVBInfo, USHORT *HDE, USHORT *VDE)
{
    switch (pVBInfo->LCDResInfo) {
    case Panel1024x768:
    case Panel1024x768x75:
	*HDE = 1024;
	*VDE = 768;
	break;

    case Panel1280x1024:
    case Panel1280x1024x75:
	*HDE = 1280;
	*VDE = 1024;
	break;

    case Panel1400x1050:
	*HDE = 1400;
	*VDE = 1050;
	break;

    default:
	*HDE = 1600;
	*VDE = 1200;
	break;
    }
}


/* --------------------------------------------------------------------- */
/* Function : XGI_GetLVDSData */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_GetLVDSData(USHORT ModeNo, USHORT ModeIdIndex,
                USHORT RefreshRateTableIndex, PVB_DEVICE_INFO pVBInfo)
{
    USHORT tempbx;
    XGI330_LVDSDataStruct *LCDPtr = NULL;

    tempbx = 2;

    if (pVBInfo->VBInfo & (SetCRT2ToLCD | SetCRT2ToLCDA)) {
        LCDPtr =
            (XGI330_LVDSDataStruct *) XGI_GetLcdPtr(tempbx, ModeNo,
                                                    ModeIdIndex,
                                                    RefreshRateTableIndex,
                                                    pVBInfo);
        pVBInfo->VGAHT = LCDPtr->VGAHT;
        pVBInfo->VGAVT = LCDPtr->VGAVT;
        pVBInfo->HT = LCDPtr->LCDHT;
        pVBInfo->VT = LCDPtr->LCDVT;
    }

    if (pVBInfo->VBInfo & (SetCRT2ToLCD | SetCRT2ToLCDA)) {
        if (!(pVBInfo->LCDInfo & (SetLCDtoNonExpanding | EnableScalingLCD))) {
	    get_HDE_VDE(pVBInfo, & pVBInfo->HDE, & pVBInfo->VDE);
        }
    }
}


/* --------------------------------------------------------------------- */
/* Function : XGI_ModCRT1Regs */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_ModCRT1Regs(USHORT ModeNo, USHORT ModeIdIndex,
                USHORT RefreshRateTableIndex,
                PXGI_HW_DEVICE_INFO HwDeviceExtension,
                PVB_DEVICE_INFO pVBInfo)
{
    UCHAR index;
    USHORT tempbx, i;
    XGI_LVDSCRT1HDataStruct *LCDPtr = NULL;
    XGI_LVDSCRT1VDataStruct *LCDPtr1 = NULL;
    /* XGI330_CHTVDataStruct *TVPtr = NULL ; */

    if (ModeNo <= 0x13)
        index = pVBInfo->SModeIDTable[ModeIdIndex].St_CRT2CRTC;
    else
        index = pVBInfo->RefIndex[RefreshRateTableIndex].Ext_CRT2CRTC;

    index = index & IndexMask;

    if ((pVBInfo->IF_DEF_ScaleLCD == 0)
        || ((pVBInfo->IF_DEF_ScaleLCD == 1)
            && (!(pVBInfo->LCDInfo & EnableScalingLCD)))) {
        tempbx = 0;

        if (pVBInfo->VBInfo & (SetCRT2ToLCD | SetCRT2ToLCDA)) {
            LCDPtr =
                (XGI_LVDSCRT1HDataStruct *) XGI_GetLcdPtr(tempbx, ModeNo,
                                                          ModeIdIndex,
                                                          RefreshRateTableIndex,
                                                          pVBInfo);

            for (i = 0; i < 8; i++)
                pVBInfo->TimingH.data[i] = LCDPtr[0].Reg[i];
        }

        XGI_SetCRT1Timing_H(pVBInfo, HwDeviceExtension);

        tempbx = 1;

        if (pVBInfo->VBInfo & (SetCRT2ToLCD | SetCRT2ToLCDA)) {
            LCDPtr1 =
                (XGI_LVDSCRT1VDataStruct *) XGI_GetLcdPtr(tempbx, ModeNo,
                                                          ModeIdIndex,
                                                          RefreshRateTableIndex,
                                                          pVBInfo);
            for (i = 0; i < 7; i++)
                pVBInfo->TimingV.data[i] = LCDPtr1[0].Reg[i];
        }

        XGI_SetCRT1Timing_V(ModeIdIndex, ModeNo, pVBInfo);
    }
}



/* --------------------------------------------------------------------- */
/* Function : XGI_SetLVDSRegs */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_SetLVDSRegs(USHORT ModeNo, USHORT ModeIdIndex,
                USHORT RefreshRateTableIndex, PVB_DEVICE_INFO pVBInfo)
{
    USHORT tempbx, tempax, tempcx, tempdx, push1, push2, modeflag;
    unsigned long temp, temp1, temp2, temp3, push3;
    XGI330_LCDDataDesStruct *LCDPtr = NULL;
    XGI330_LCDDataDesStruct2 *LCDPtr1 = NULL;

    if (ModeNo > 0x13)
        modeflag = pVBInfo->EModeIDTable[ModeIdIndex].Ext_ModeFlag;
    else
        modeflag = pVBInfo->SModeIDTable[ModeIdIndex].St_ModeFlag;

    if (!(pVBInfo->SetFlag & Win9xDOSMode)) {
        if (pVBInfo->IF_DEF_OEMUtil == 1) {
            tempbx = 8;
            LCDPtr =
                (XGI330_LCDDataDesStruct *) XGI_GetLcdPtr(tempbx, ModeNo,
                                                          ModeIdIndex,
                                                          RefreshRateTableIndex,
                                                          pVBInfo);
        }

        if ((pVBInfo->IF_DEF_OEMUtil == 0) || (LCDPtr == 0)) {
            tempbx = 3;
            if (pVBInfo->LCDInfo & EnableScalingLCD)
                LCDPtr1 =
                    (XGI330_LCDDataDesStruct2 *) XGI_GetLcdPtr(tempbx, ModeNo,
                                                               ModeIdIndex,
                                                               RefreshRateTableIndex,
                                                               pVBInfo);
            else
                LCDPtr =
                    (XGI330_LCDDataDesStruct *) XGI_GetLcdPtr(tempbx, ModeNo,
                                                              ModeIdIndex,
                                                              RefreshRateTableIndex,
                                                              pVBInfo);
        }

        XGI_GetLCDSync(&tempax, &tempbx, pVBInfo);
        push1 = tempbx;
        push2 = tempax;

        /* GetLCDResInfo */
        if (pVBInfo->LCDInfo & SetLCDtoNonExpanding) {
	    get_HDE_VDE(pVBInfo, & pVBInfo->HDE, & pVBInfo->VDE);

            pVBInfo->VGAHDE = pVBInfo->HDE;
            pVBInfo->VGAVDE = pVBInfo->VDE;
        }

        tempax = pVBInfo->HT;

	tempbx = (pVBInfo->LCDInfo & EnableScalingLCD)
	    ? LCDPtr1->LCDHDES : LCDPtr->LCDHDES;

        tempcx = pVBInfo->HDE;
        tempbx = tempbx & 0x0fff;
        tempcx += tempbx;

        if (tempcx >= tempax)
            tempcx -= tempax;

        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x1A, tempbx & 0x07);

        tempcx = tempcx >> 3;
        tempbx = tempbx >> 3;

        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x16,
                   (USHORT) (tempbx & 0xff));
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x17,
                   (USHORT) (tempcx & 0xff));

        tempax = pVBInfo->HT;

        if (pVBInfo->LCDInfo & EnableScalingLCD)
            tempbx = LCDPtr1->LCDHRS;
        else
            tempbx = LCDPtr->LCDHRS;

        tempcx = push2;

        if (pVBInfo->LCDInfo & EnableScalingLCD)
            tempcx = LCDPtr1->LCDHSync;

        tempcx += tempbx;

        if (tempcx >= tempax)
            tempcx -= tempax;

	/* FIXME: Won't this *always* set tempax to zero? */
        tempax = tempbx & 0x07;
        tempax = tempax >> 5;
        tempcx = tempcx >> 3;
        tempbx = tempbx >> 3;

        tempcx &= 0x1f;
        tempax |= tempcx;

        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x15, tempax);
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x14,
                   (USHORT) (tempbx & 0xff));

        tempax = pVBInfo->VT;
        if (pVBInfo->LCDInfo & EnableScalingLCD)
            tempbx = LCDPtr1->LCDVDES;
        else
            tempbx = LCDPtr->LCDVDES;
        tempcx = pVBInfo->VDE;

        tempbx = tempbx & 0x0fff;
        tempcx += tempbx;
        if (tempcx >= tempax)
            tempcx -= tempax;

        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x1b,
                   (USHORT) (tempbx & 0xff));
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x1c,
                   (USHORT) (tempcx & 0xff));

        tempbx = (tempbx >> 8) & 0x07;
        tempcx = (tempcx >> 8) & 0x07;

        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x1d,
                   (USHORT) ((tempcx << 3) | tempbx));

        tempax = pVBInfo->VT;
        if (pVBInfo->LCDInfo & EnableScalingLCD)
            tempbx = LCDPtr1->LCDVRS;
        else
            tempbx = LCDPtr->LCDVRS;

        /* tempbx = tempbx >> 4 ; */
        tempcx = push1;

        if (pVBInfo->LCDInfo & EnableScalingLCD)
            tempcx = LCDPtr1->LCDVSync;

        tempcx += tempbx;
        if (tempcx >= tempax)
            tempcx -= tempax;

        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x18,
                   (USHORT) (tempbx & 0xff));
        XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part1Port, 0x19, ~0x0f,
                        (USHORT) (tempcx & 0x0f));

        tempax = ((tempbx >> 8) & 0x07) << 3;

        tempbx = pVBInfo->VGAVDE;
        if (tempbx != pVBInfo->VDE)
            tempax |= 0x40;

        if (pVBInfo->LCDInfo & EnableLVDSDDA)
            tempax |= 0x40;

        XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part1Port, 0x1a, 0x07,
                        tempax);

        tempcx = pVBInfo->VGAVT;
        tempbx = pVBInfo->VDE;
        tempax = pVBInfo->VGAVDE;
        tempcx -= tempax;

        temp = tempax;          /* 0430 ylshieh */
        temp1 = (temp << 18) / tempbx;

        tempdx = (USHORT) ((temp << 18) % tempbx);

        if (tempdx != 0)
            temp1 += 1;

        temp2 = temp1;
        push3 = temp2;

        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x37,
                   (USHORT) (temp2 & 0xff));
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x36,
                   (USHORT) ((temp2 >> 8) & 0xff));

        tempbx = (USHORT) (temp2 >> 16);
        tempax = tempbx & 0x03;

        tempbx = pVBInfo->VGAVDE;
        if (tempbx == pVBInfo->VDE)
            tempax |= 0x04;

        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x35, tempax);

        if (pVBInfo->VBType & VB_XGI301C) {
            temp2 = push3;
            XGI_SetReg((XGIIOADDRESS) pVBInfo->Part4Port, 0x3c,
                       (USHORT) (temp2 & 0xff));
            XGI_SetReg((XGIIOADDRESS) pVBInfo->Part4Port, 0x3b,
                       (USHORT) ((temp2 >> 8) & 0xff));
            tempbx = (USHORT) (temp2 >> 16);
            XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part4Port, 0x3a, ~0xc0,
                            (USHORT) ((tempbx & 0xff) << 6));

            tempcx = pVBInfo->VGAVDE;
            if (tempcx == pVBInfo->VDE)
                XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part4Port, 0x30,
                                ~0x0c, 0x00);
            else
                XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part4Port, 0x30,
                                ~0x0c, 0x08);
        }

        tempcx = pVBInfo->VGAHDE;
        tempbx = pVBInfo->HDE;

        temp1 = tempcx << 16;

        tempax = (USHORT) (temp1 / tempbx);

        if ((tempbx & 0xffff) == (tempcx & 0xffff))
            tempax = 65535;

        temp3 = tempax;
        temp1 = pVBInfo->VGAHDE << 16;

        temp1 /= temp3;
        temp3 = temp3 << 16;
        temp1 -= 1;

        temp3 = (temp3 & 0xffff0000) + (temp1 & 0xffff);

        tempax = (USHORT) (temp3 & 0xff);
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x1f, tempax);

        temp1 = pVBInfo->VGAVDE << 18;
        temp1 = temp1 / push3;
        tempbx = (USHORT) (temp1 & 0xffff);

        if (pVBInfo->LCDResInfo == Panel1024x768)
            tempbx -= 1;

        tempax = ((tempbx >> 8) & 0xff) << 3;
        tempax |= (USHORT) ((temp3 >> 8) & 0x07);
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x20,
                   (USHORT) (tempax & 0xff));
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x21,
                   (USHORT) (tempbx & 0xff));

        temp3 = temp3 >> 16;

        if (modeflag & HalfDCLK)
            temp3 = temp3 >> 1;

        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x22,
                   (USHORT) ((temp3 >> 8) & 0xff));
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x23,
                   (USHORT) (temp3 & 0xff));
    }
}


/* --------------------------------------------------------------------- */
/* Function : XGI_SetCRT2ECLK */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_SetCRT2ECLK(USHORT ModeNo, USHORT ModeIdIndex,
                USHORT RefreshRateTableIndex, PVB_DEVICE_INFO pVBInfo)
{
    UCHAR di[2];
    int i;
    const unsigned vclkindex =
        XGI_GetVCLKPtr(RefreshRateTableIndex, ModeNo, ModeIdIndex, pVBInfo);

    XGI_GetVCLKLen(vclkindex, di, pVBInfo);
    XGI_GetLCDVCLKPtr(di, pVBInfo);

    for (i = 0; i < 4; i++) {
        XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->P3d4, 0x31, ~0x30,
                        (USHORT) (0x10 * i));
        if ((!(pVBInfo->VBInfo & SetCRT2ToLCDA))
            && (!(pVBInfo->VBInfo & SetInSlaveMode))) {
            XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x2e, di[0]);
            XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x2f, di[1]);
        }
        else {
            XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x2b, di[0]);
            XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x2c, di[1]);
        }
    }
}


/* --------------------------------------------------------------------- */
/* Function : XGI_UpdateModeInfo */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_UpdateModeInfo(PXGI_HW_DEVICE_INFO HwDeviceExtension,
                   PVB_DEVICE_INFO pVBInfo)
{
    USHORT tempcl, tempch, temp, tempbl, tempax;

    if (pVBInfo->
        VBType & (VB_XGI301B | VB_XGI302B | VB_XGI301LV | VB_XGI302LV |
                  VB_XGI301C)) {
        tempcl = 0;
        tempch = 0;
        temp = XGI_GetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x01);

        if (!(temp & 0x20)) {
            temp = XGI_GetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x17);
            if (temp & 0x80) {
                if ((HwDeviceExtension->jChipType == XG20)
                    || (HwDeviceExtension->jChipType >= XG40))
                    temp = XGI_GetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x53);
                else
                    temp = XGI_GetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x63);

                if (!(temp & 0x40))
                    tempcl |= ActiveCRT1;
            }
        }

        temp = XGI_GetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x2e);
        temp &= 0x0f;

        if (!(temp == 0x08)) {
            tempax = XGI_GetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x13);       /* Check ChannelA by Part1_13 [2003/10/03] */
            if (tempax & 0x04)
                tempcl = tempcl | ActiveLCD;

            temp &= 0x05;

            if (!(tempcl & ActiveLCD))
                if (temp == 0x01)
                    tempcl |= ActiveCRT2;

            if (temp == 0x04)
                tempcl |= ActiveLCD;

            if (temp == 0x05) {
                temp = XGI_GetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x00);

                if (!(temp & 0x08))
                    tempch |= ActiveAVideo;

                if (!(temp & 0x04))
                    tempch |= ActiveSVideo;

                if (temp & 0x02)
                    tempch |= ActiveSCART;

                if (pVBInfo->VBInfo & SetCRT2ToHiVisionTV) {
                    if (temp & 0x01)
                        tempch |= ActiveHiTV;
                }

                if (pVBInfo->VBInfo & SetCRT2ToYPbPr) {
                    temp =
                        XGI_GetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x4d);

                    if (temp & 0x10)
                        tempch |= ActiveYPbPr;
                }

                if (tempch != 0)
                    tempcl |= ActiveTV;
            }
        }

        temp = XGI_GetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x3d);
        if (tempcl & ActiveLCD) {
            if ((pVBInfo->SetFlag & ReserveTVOption)) {
                if (temp & ActiveTV)
                    tempcl |= ActiveTV;
            }
        }
        temp = tempcl;
        tempbl = ~ModeSwitchStatus;
        XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->P3d4, 0x3d, tempbl, temp);

        if (!(pVBInfo->SetFlag & ReserveTVOption))
            XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x3e, tempch);
    }
    else {
        return;
    }
}


/* --------------------------------------------------------------------- */
/* Function : XGI_GetVBType */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_GetVBType(PVB_DEVICE_INFO pVBInfo)
{
    USHORT flag, tempbx, tempah;

    tempbx = VB_XGI302B;
    flag = XGI_GetReg((XGIIOADDRESS) pVBInfo->Part4Port, 0x00);
    if (flag != 0x02) {
	tempbx = VB_XGI301;
	flag = XGI_GetReg((XGIIOADDRESS) pVBInfo->Part4Port, 0x01);
	if (flag >= 0xB0) {
	    tempbx = VB_XGI301B;
	    if (flag >= 0xC0) {
		tempbx = VB_XGI301C;
		if (flag >= 0xD0) {
		    tempbx = VB_XGI301LV;
		    if (flag >= 0xE0) {
			tempbx = VB_XGI302LV;
			tempah = XGI_GetReg((XGIIOADDRESS) pVBInfo->Part4Port,
					    0x39);
			if (tempah != 0xFF)
			    tempbx = VB_XGI301C;
		    }
		}
	    }

	    if (tempbx & (VB_XGI301B | VB_XGI302B)) {
		flag = XGI_GetReg((XGIIOADDRESS) pVBInfo->Part4Port, 0x23);

		if (!(flag & 0x02))
		    tempbx = tempbx | VB_NoLCD;
	    }
	}
    }

    pVBInfo->VBType = tempbx;
}


/* --------------------------------------------------------------------- */
/* Function : XGI_GetVBInfo */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_GetVBInfo(USHORT ModeNo, USHORT ModeIdIndex,
              PXGI_HW_DEVICE_INFO HwDeviceExtension, PVB_DEVICE_INFO pVBInfo)
{
    USHORT tempax, push, tempbx, temp, modeflag;

    if (ModeNo <= 0x13) {
        modeflag = pVBInfo->SModeIDTable[ModeIdIndex].St_ModeFlag;
    }
    else {
        modeflag = pVBInfo->EModeIDTable[ModeIdIndex].Ext_ModeFlag;
    }

    pVBInfo->SetFlag = 0;
    pVBInfo->ModeType = modeflag & ModeInfoFlag;
    tempbx = 0;

    if (pVBInfo->VBType & 0xFFFF) {
        temp = XGI_GetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x30);  /* Check Display Device */
        tempbx = tempbx | temp;
        temp = XGI_GetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x31);
        push = temp;
        push = push << 8;
        tempax = temp << 8;
        tempbx = tempbx | tempax;
        temp =
            (SetCRT2ToDualEdge | SetCRT2ToYPbPr | SetCRT2ToLCDA |
             SetInSlaveMode | DisableCRT2Display);
        temp = 0xFFFF ^ temp;
        tempbx &= temp;

        temp = XGI_GetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x38);

        if (pVBInfo->IF_DEF_LCDA == 1) {
            /* if ( ( pVBInfo->VBType & VB_XGI302B ) || ( pVBInfo->VBType & VB_XGI301LV ) || ( pVBInfo->VBType & VB_XGI302LV ) || ( pVBInfo->VBType & VB_XGI301C ) ) */
            if (pVBInfo->
                VBType & (VB_XGI302B | VB_XGI301LV | VB_XGI302LV |
                          VB_XGI301C)) {
                if (temp & EnableDualEdge) {
                    tempbx |= SetCRT2ToDualEdge;

                    if (temp & SetToLCDA)
                        tempbx |= SetCRT2ToLCDA;
                }
            }
        }

        if (pVBInfo->IF_DEF_YPbPr == 1) {
            if ((pVBInfo->VBType & VB_XGI301LV)
                || (pVBInfo->VBType & VB_XGI302LV)
                || (pVBInfo->VBType & VB_XGI301C)) {
                if (temp & SetYPbPr) {  /* temp = CR38 */
                    if (pVBInfo->IF_DEF_HiVision == 1) {
                        temp = XGI_GetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x35);  /* shampoo add for new scratch */
                        temp &= YPbPrMode;
                        tempbx |= SetCRT2ToHiVisionTV;

                        if (temp != YPbPrMode1080i) {
                            tempbx &= (~SetCRT2ToHiVisionTV);
                            tempbx |= SetCRT2ToYPbPr;
                        }
                    }

                    /* tempbx |= SetCRT2ToYPbPr ; */
                }
            }
        }

        tempax = push;          /* restore CR31 */

        if (pVBInfo->IF_DEF_YPbPr == 1) {
            if (pVBInfo->IF_DEF_HiVision == 1)
                temp = 0x09FC;
            else
                temp = 0x097C;
        }
        else {
            if (pVBInfo->IF_DEF_HiVision == 1)
                temp = 0x01FC;
            else
                temp = 0x017C;
        }


        if (!(tempbx & temp)) {
            tempax |= DisableCRT2Display;
            tempbx = 0;
        }

        if (pVBInfo->IF_DEF_LCDA == 1) {        /* Select Display Device */
            if (!(pVBInfo->VBType & VB_NoLCD)) {
                if (tempbx & SetCRT2ToLCDA) {
                    if (tempbx & SetSimuScanMode)
                        tempbx &=
                            (~
                             (SetCRT2ToLCD | SetCRT2ToRAMDAC | SwitchToCRT2));
                    else
                        tempbx &=
                            (~
                             (SetCRT2ToLCD | SetCRT2ToRAMDAC | SetCRT2ToTV |
                              SwitchToCRT2));
                }
            }
        }

        /* shampoo add */
        if (!(tempbx & (SwitchToCRT2 | SetSimuScanMode))) {     /* for driver abnormal */
            if (pVBInfo->IF_DEF_CRT2Monitor == 1) {
                if (tempbx & SetCRT2ToRAMDAC) {
                    tempbx &=
                        (0xFF00 | SetCRT2ToRAMDAC | SwitchToCRT2 |
                         SetSimuScanMode);
                    tempbx &= (0x00FF | (~SetCRT2ToYPbPr));
                }
            }
            else
                tempbx &= (~(SetCRT2ToRAMDAC | SetCRT2ToLCD | SetCRT2ToTV));
        }

        if (!(pVBInfo->VBType & VB_NoLCD)) {
            if (tempbx & SetCRT2ToLCD) {
                tempbx &=
                    (0xFF00 | SetCRT2ToLCD | SwitchToCRT2 | SetSimuScanMode);
                tempbx &= (0x00FF | (~SetCRT2ToYPbPr));
            }
        }

        if (tempbx & SetCRT2ToSCART) {
            tempbx &=
                (0xFF00 | SetCRT2ToSCART | SwitchToCRT2 | SetSimuScanMode);
            tempbx &= (0x00FF | (~SetCRT2ToYPbPr));
        }

        if (pVBInfo->IF_DEF_YPbPr == 1) {
            if (tempbx & SetCRT2ToYPbPr)
                tempbx &= (0xFF00 | SwitchToCRT2 | SetSimuScanMode);
        }

        if (pVBInfo->IF_DEF_HiVision == 1) {
            if (tempbx & SetCRT2ToHiVisionTV)
                tempbx &=
                    (0xFF00 | SetCRT2ToHiVisionTV | SwitchToCRT2 |
                     SetSimuScanMode);
        }

        if (tempax & DisableCRT2Display) {      /* Set Display Device Info */
            if (!(tempbx & (SwitchToCRT2 | SetSimuScanMode)))
                tempbx = DisableCRT2Display;
        }

        if (!(tempbx & DisableCRT2Display)) {
            if ((!(tempbx & DriverMode)) || (!(modeflag & CRT2Mode))) {
                if (pVBInfo->IF_DEF_LCDA == 1) {
                    if (!(tempbx & SetCRT2ToLCDA))
                        tempbx |= (SetInSlaveMode | SetSimuScanMode);
                }

                if (pVBInfo->IF_DEF_VideoCapture == 1) {
                    if ((HwDeviceExtension->jChipType >= XG40)
                        && (HwDeviceExtension->jChipType <= XG45)) {
                        if (ModeNo <= 13) {
                            /* CRT2 not need to support */
                            if (!(tempbx & SetCRT2ToRAMDAC)) {
                                tempbx &= (0x00FF | (~SetInSlaveMode));
                                pVBInfo->SetFlag |= EnableVCMode;
                            }
                        }
                    }
                }
            }

            /*LCD+TV can't support in slave mode (Force LCDA+TV->LCDB) */
            if ((tempbx & SetInSlaveMode) && (tempbx & SetCRT2ToLCDA)) {
                tempbx ^= (SetCRT2ToLCD | SetCRT2ToLCDA | SetCRT2ToDualEdge);
                pVBInfo->SetFlag |= ReserveTVOption;
            }
        }
    }

    pVBInfo->VBInfo = tempbx;
}


/* --------------------------------------------------------------------- */
/* Function : XGI_GetTVInfo */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_GetTVInfo(USHORT ModeNo, USHORT ModeIdIndex, PVB_DEVICE_INFO pVBInfo)
{
    USHORT temp, tempbx = 0, resinfo = 0, modeflag, index1;

    tempbx = 0;
    resinfo = 0;

    if (pVBInfo->VBInfo & SetCRT2ToTV) {
        if (ModeNo <= 0x13) {
            modeflag = pVBInfo->SModeIDTable[ModeIdIndex].St_ModeFlag;  /* si+St_ModeFlag */
            resinfo = pVBInfo->SModeIDTable[ModeIdIndex].St_ResInfo;    /* si+St_ResInfo */
        }
        else {
            modeflag = pVBInfo->EModeIDTable[ModeIdIndex].Ext_ModeFlag;
            resinfo = pVBInfo->EModeIDTable[ModeIdIndex].Ext_RESINFO;   /* si+Ext_ResInfo */
        }

        if (pVBInfo->VBInfo & SetCRT2ToTV) {
            temp = XGI_GetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x35);
            tempbx = temp;
            if (tempbx & SetPALTV) {
                tempbx &=
                    (SetCHTVOverScan | SetPALMTV | SetPALNTV | SetPALTV);
                if (tempbx & SetPALMTV)
                    tempbx &= ~SetPALTV;        /* set to NTSC if PAL-M */
            }
            else
                tempbx &= (SetCHTVOverScan | SetNTSCJ | SetPALTV);
        }

        if (pVBInfo->VBInfo & SetCRT2ToSCART)
            tempbx |= SetPALTV;

        if (pVBInfo->IF_DEF_YPbPr == 1) {
            if (pVBInfo->VBInfo & SetCRT2ToYPbPr) {
                index1 = XGI_GetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x35);
                index1 &= YPbPrMode;

                if (index1 == YPbPrMode525i)
                    tempbx |= SetYPbPrMode525i;

                if (index1 == YPbPrMode525p)
                    tempbx = tempbx | SetYPbPrMode525p;
                if (index1 == YPbPrMode750p)
                    tempbx = tempbx | SetYPbPrMode750p;
            }
        }

        if (pVBInfo->IF_DEF_HiVision == 1) {
            if (pVBInfo->VBInfo & SetCRT2ToHiVisionTV) {
                tempbx = tempbx | SetYPbPrMode1080i | SetPALTV;
            }
        }

        if ((pVBInfo->VBInfo & SetInSlaveMode)
            && (!(pVBInfo->VBInfo & SetNotSimuMode)))
            tempbx |= TVSimuMode;

        if (!(tempbx & SetPALTV) && (modeflag > 13) && (resinfo == 8))  /* NTSC 1024x768, */
            tempbx |= NTSC1024x768;

        tempbx |= RPLLDIV2XO;

        if (pVBInfo->VBInfo & SetCRT2ToHiVisionTV) {
            if (pVBInfo->VBInfo & SetInSlaveMode)
                tempbx &= (~RPLLDIV2XO);
        }
        else {
            if (tempbx & (SetYPbPrMode525p | SetYPbPrMode750p))
                tempbx &= (~RPLLDIV2XO);
            else if (!
                     (pVBInfo->
                      VBType & (VB_XGI301B | VB_XGI302B | VB_XGI301LV |
                                VB_XGI302LV | VB_XGI301C))) {
                if (tempbx & TVSimuMode)
                    tempbx &= (~RPLLDIV2XO);
            }
        }
    }
    pVBInfo->TVInfo = tempbx;
}


/* --------------------------------------------------------------------- */
/* Function : XGI_GetLCDInfo */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
BOOLEAN
XGI_GetLCDInfo(USHORT ModeNo, USHORT ModeIdIndex, PVB_DEVICE_INFO pVBInfo)
{
    USHORT temp, tempax, tempbx, modeflag, resinfo = 0, LCDIdIndex;

    pVBInfo->LCDResInfo = 0;
    pVBInfo->LCDTypeInfo = 0;
    pVBInfo->LCDInfo = 0;

    if (ModeNo <= 0x13) {
        modeflag = pVBInfo->SModeIDTable[ModeIdIndex].St_ModeFlag;      /* si+St_ModeFlag // */
    }
    else {
        modeflag = pVBInfo->EModeIDTable[ModeIdIndex].Ext_ModeFlag;
        resinfo = pVBInfo->EModeIDTable[ModeIdIndex].Ext_RESINFO;       /* si+Ext_ResInfo// */
    }

    temp = XGI_GetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x36);      /* Get LCD Res.Info */
    tempbx = temp & 0x0F;

    if (tempbx == 0)
        tempbx = Panel1024x768; /* default */

    /* LCD75 [2003/8/22] Vicent */
    if ((tempbx == Panel1024x768) || (tempbx == Panel1280x1024)) {
        if (pVBInfo->VBInfo & DriverMode) {
            tempax = XGI_GetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x33);
            if (pVBInfo->VBInfo & SetCRT2ToLCDA)
                tempax &= 0x0F;
            else
                tempax = tempax >> 4;

            if ((resinfo == 6) || (resinfo == 9)) {
                if (tempax >= 3)
                    tempbx |= PanelRef75Hz;
            }
            else if ((resinfo == 7) || (resinfo == 8)) {
                if (tempax >= 4)
                    tempbx |= PanelRef75Hz;
            }
        }
    }

    pVBInfo->LCDResInfo = tempbx;

    /* End of LCD75 */

    if (pVBInfo->IF_DEF_OEMUtil == 1) {
        pVBInfo->LCDTypeInfo = (temp & 0xf0) >> 4;
    }

    if (!(pVBInfo->VBInfo & (SetCRT2ToLCD | SetCRT2ToLCDA))) {
        return 0;
    }

    tempbx = 0;

    temp = XGI_GetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x37);

    temp &= (ScalingLCD | LCDNonExpanding | LCDSyncBit | SetPWDEnable);

    if ((pVBInfo->IF_DEF_ScaleLCD == 1) && (temp & LCDNonExpanding))
        temp &= ~EnableScalingLCD;

    tempbx |= temp;

    LCDIdIndex = XGI_GetLCDCapPtr1(pVBInfo);

    tempax = pVBInfo->LCDCapList[LCDIdIndex].LCD_Capability;

    if (((pVBInfo->VBType & VB_XGI302LV) || (pVBInfo->VBType & VB_XGI301C))
        && (tempax & LCDDualLink)) {
        tempbx |= SetLCDDualLink;
    }

    if ((pVBInfo->LCDResInfo == Panel1400x1050)
        && (pVBInfo->VBInfo & SetCRT2ToLCD) && (ModeNo > 0x13)
        && (resinfo == 9) && (!(tempbx & EnableScalingLCD)))
        tempbx |= SetLCDtoNonExpanding; /* set to center in 1280x1024 LCDB for Panel1400x1050 */

/*
    if ( tempax & LCDBToA )
    {
        tempbx |= SetLCDBToA ;
    }
*/

    if (pVBInfo->IF_DEF_ExpLink == 1) {
        if (modeflag & HalfDCLK) {
            /* if ( !( pVBInfo->LCDInfo&LCDNonExpanding ) ) */
            if (!(tempbx & SetLCDtoNonExpanding)) {
                tempbx |= EnableLVDSDDA;
            }
            else {
                if (ModeNo > 0x13) {
                    if (pVBInfo->LCDResInfo == Panel1024x768) {
                        if (resinfo == 4) {     /* 512x384  */
                            tempbx |= EnableLVDSDDA;
                        }
                    }
                }
            }
        }
    }

    if (pVBInfo->VBInfo & SetInSlaveMode) {
        if (pVBInfo->VBInfo & SetNotSimuMode) {
            tempbx |= LCDVESATiming;
        }
    }
    else {
        tempbx |= LCDVESATiming;
    }

    temp = XGI_GetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x39);
    if (temp & ReduceTiming) {
        tempbx |= EnableReduceTiming;
    }

    pVBInfo->LCDInfo = tempbx;

    if (pVBInfo->IF_DEF_PWD == 1) {
        if (pVBInfo->LCDInfo & SetPWDEnable) {
            if ((pVBInfo->VBType & VB_XGI302LV)
                || (pVBInfo->VBType & VB_XGI301C)) {
                if (!(tempax & PWDEnable)) {
                    pVBInfo->LCDInfo &= ~SetPWDEnable;
                }
            }
        }
    }

    {
        if (tempax & (LockLCDBToA | StLCDBToA)) {
            if (pVBInfo->VBInfo & SetInSlaveMode) {
                if (!(tempax & LockLCDBToA)) {
                    if (ModeNo <= 0x13) {
                        pVBInfo->VBInfo &=
                            ~(SetSimuScanMode | SetInSlaveMode |
                              SetCRT2ToLCD);
                        pVBInfo->VBInfo |= SetCRT2ToLCDA | SetCRT2ToDualEdge;
                    }
                }
            }
        }
    }

    return (1);
}


/* --------------------------------------------------------------------- */
/* Function : */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
BOOLEAN
XGINew_CheckMemorySize(PXGI_HW_DEVICE_INFO HwDeviceExtension, USHORT ModeNo,
                       USHORT ModeIdIndex, PVB_DEVICE_INFO pVBInfo)
{
    USHORT memorysize, modeflag, temp, temp1, tmp;

    if (ModeNo <= 0x13) {
        modeflag = pVBInfo->SModeIDTable[ModeIdIndex].St_ModeFlag;
    }
    else {
        modeflag = pVBInfo->EModeIDTable[ModeIdIndex].Ext_ModeFlag;
    }

    /* ModeType = modeflag&ModeInfoFlag ; // Get mode type */

    memorysize = modeflag & MemoryInfoFlag;
    memorysize = memorysize > MemorySizeShift;
    memorysize++;               /* Get memory size */

    temp = XGI_GetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x14);      /* Get DRAM Size */
    tmp = temp;

    if (HwDeviceExtension->jChipType == XG40) {
        temp = 1 << ((temp & 0x0F0) >> 4);      /* memory size per channel SR14[7:4] */
        if ((tmp & 0x0c) == 0x0C) {     /* Qual channels */
            temp <<= 2;
        }
        else if ((tmp & 0x0c) == 0x08) {        /* Dual channels */
            temp <<= 1;
        }
    }
    else if (HwDeviceExtension->jChipType == XG42) {
        temp = 1 << ((temp & 0x0F0) >> 4);      /* memory size per channel SR14[7:4] */
        if ((tmp & 0x04) == 0x04) {     /* Dual channels */
            temp <<= 1;
        }
    }
    else if (HwDeviceExtension->jChipType == XG45) {
        temp = 1 << ((temp & 0x0F0) >> 4);      /* memory size per channel SR14[7:4] */
        if ((tmp & 0x0c) == 0x0C) {     /* Qual channels */
            temp <<= 2;
        }
        else if ((tmp & 0x0c) == 0x08) {        /* triple channels */
            temp1 = temp;
            temp <<= 1;
            temp += temp1;
        }
        else if ((tmp & 0x0c) == 0x04) {        /* Dual channels */
            temp <<= 1;
        }
    }
    if (temp < memorysize)
        return (FALSE);
    else
        return (TRUE);
}


/* --------------------------------------------------------------------- */
/* Function : XGI_DisplayOn */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_DisplayOn(PVB_DEVICE_INFO pVBInfo)
{
    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->P3c4, 0x01, 0xDF, 0x00);
}


/* --------------------------------------------------------------------- */
/* Function : XGI_DisplayOff */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_DisplayOff(PVB_DEVICE_INFO pVBInfo)
{
    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->P3c4, 0x01, 0xDF, 0x20);
}


/**
 * Wait for vertical or horizontal blanking period.
 */
void
XGI_WaitDisplay(PVB_DEVICE_INFO pVBInfo)
{
    while ((XGI_GetRegByte((XGIIOADDRESS) pVBInfo->P3da) & 0x01))
        break;

    while (!(XGI_GetRegByte((XGIIOADDRESS) pVBInfo->P3da) & 0x01))
        break;
}


/* --------------------------------------------------------------------- */
/* Function : XGI_SenseCRT1 */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */

void
XGI_SenseCRT1(PVB_DEVICE_INFO pVBInfo)
{
    UCHAR CRTCData[17] = { 0x5F, 0x4F, 0x50, 0x82, 0x55, 0x81,
        0x0B, 0x3E, 0xE9, 0x0B, 0xDF, 0xE7,
        0x04, 0x00, 0x00, 0x05, 0x00
    };

    UCHAR SR01 = 0, SR1F = 0, SR07 = 0, SR06 = 0;

    UCHAR CR17, CR63, SR31;
    USHORT temp;
    UCHAR DAC_TEST_PARMS[3] = { 0x0F, 0x0F, 0x0F };

    int i;
#ifndef LINUX_XF86
    int j;
#endif

    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x05, 0x86);

    /* [2004/05/06] Vicent to fix XG42 single LCD sense to CRT+LCD */
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x57, 0x4A);
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x53,
               (UCHAR) (XGI_GetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x53) |
                        0x02));

    SR31 = (UCHAR) XGI_GetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x31);
    CR63 = (UCHAR) XGI_GetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x63);
    SR01 = (UCHAR) XGI_GetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x01);

    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x01, (UCHAR) (SR01 & 0xDF));
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x63, (UCHAR) (CR63 & 0xBF));

    CR17 = (UCHAR) XGI_GetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x17);
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x17, (UCHAR) (CR17 | 0x80));

    SR1F = (UCHAR) XGI_GetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x1F);
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x1F, (UCHAR) (SR1F | 0x04));

    SR07 = (UCHAR) XGI_GetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x07);
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x07, (UCHAR) (SR07 & 0xFB));
    SR06 = (UCHAR) XGI_GetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x06);
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x06, (UCHAR) (SR06 & 0xC3));

    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x11, 0x00);

    for (i = 0; i < 8; i++)
        XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4, (USHORT) i, CRTCData[i]);

    for (i = 8; i < 11; i++)
        XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4, (USHORT) (i + 8),
                   CRTCData[i]);

    for (i = 11; i < 13; i++)
        XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4, (USHORT) (i + 4),
                   CRTCData[i]);

    for (i = 13; i < 16; i++)
        XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4, (USHORT) (i - 3),
                   CRTCData[i]);

    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x0E,
               (UCHAR) (CRTCData[16] & 0xE0));

    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x31, 0x00);
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x2B, 0x1B);
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x2C, 0xE1);

    XGI_SetRegByte((XGIIOADDRESS) pVBInfo->P3c8, 0x00);
    for (i = 0; i < 256; i++) {
        XGI_SetRegByte((XGIIOADDRESS) (USHORT) (pVBInfo->P3c8 + 1),
                       (UCHAR) DAC_TEST_PARMS[0]);
        XGI_SetRegByte((XGIIOADDRESS) (USHORT) (pVBInfo->P3c8 + 1),
                       (UCHAR) DAC_TEST_PARMS[1]);
        XGI_SetRegByte((XGIIOADDRESS) (USHORT) (pVBInfo->P3c8 + 1),
                       (UCHAR) DAC_TEST_PARMS[2]);
    }

    XGI_VBLongWait(pVBInfo);
    XGI_VBLongWait(pVBInfo);
    XGI_VBLongWait(pVBInfo);

    XGINew_LCD_Wait_Time(0x01, pVBInfo);
    XGI_WaitDisplay(pVBInfo);

    temp = XGI_GetRegByte((XGIIOADDRESS) pVBInfo->P3c2);
    if (temp & 0x10) {
        XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->P3d4, 0x32, 0xDF, 0x20);
    }
    else {
        XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->P3d4, 0x32, 0xDF, 0x00);
    }

    /* alan, avoid display something, set BLACK DAC if not restore DAC */
    XGI_SetRegByte((XGIIOADDRESS) pVBInfo->P3c8, 0x00);

    for (i = 0; i < 256; i++) {
        XGI_SetRegByte((XGIIOADDRESS) (USHORT) (pVBInfo->P3c8 + 1), 0);
        XGI_SetRegByte((XGIIOADDRESS) (USHORT) (pVBInfo->P3c8 + 1), 0);
        XGI_SetRegByte((XGIIOADDRESS) (USHORT) (pVBInfo->P3c8 + 1), 0);
    }

    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x01, SR01);
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x63, CR63);
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x31, SR31);

    /* [2004/05/11] Vicent */
    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x53,
               (UCHAR) (XGI_GetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x53) &
                        0xFD));
}


/* --------------------------------------------------------------------- */
/* Function : */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
BOOLEAN
CheckDualChip(PVB_DEVICE_INFO pVBInfo)
{
    /* Check H/W trap that 2nd chip is present or not. */
    return ((BOOLEAN)
            (XGI_GetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x3A) &
             XGI_MASK_DUAL_CHIP));
}



/* --------------------------------------------------------------------- */
/* Function : SetDualChipRegs */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
SetDualChipRegs(PXGI_HW_DEVICE_INFO HwDeviceExtension,
                PVB_DEVICE_INFO pVBInfo)
{

#ifdef LINUX_XF86
    USHORT BaseAddr2nd = (USHORT) (ULONG) HwDeviceExtension->pj2ndIOAddress;
#else
    USHORT BaseAddr2nd = (USHORT) HwDeviceExtension->pj2ndIOAddress;
#endif
    USHORT XGINew_P3CC = pVBInfo->BaseAddr + MISC_OUTPUT_REG_READ_PORT;
    USHORT XGINew_2ndP3CE = BaseAddr2nd + GRAPH_ADDRESS_PORT;
    USHORT XGINew_2ndP3C4 = BaseAddr2nd + SEQ_ADDRESS_PORT;
    USHORT XGINew_2ndP3C2 = BaseAddr2nd + MISC_OUTPUT_REG_WRITE_PORT;
    UCHAR tempal, i;
    pVBInfo->BaseAddr = (USHORT) HwDeviceExtension->pjIOAddress;
    for (i = 0x00; i <= 0x04; i++) {    /* SR0 - SR4 */
        tempal = (UCHAR) XGI_GetReg((XGIIOADDRESS) pVBInfo->P3c4, i);
        XGI_SetReg((XGIIOADDRESS) XGINew_2ndP3C4, i, tempal);
    }
    for (i = 0x00; i <= 0x08; i++) {    /* GR0 - GR8 */
        tempal = (UCHAR) XGI_GetReg((XGIIOADDRESS) pVBInfo->P3ce, i);
        XGI_SetReg((XGIIOADDRESS) XGINew_2ndP3CE, i, tempal);
    }
    /* OpenKey in 2nd chip */
    XGI_SetReg((XGIIOADDRESS) XGINew_2ndP3C4, 0x05, 0x86);

    /* Copy SR06 to 2nd chip */
    tempal = (UCHAR) XGI_GetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x06);
    XGI_SetReg((XGIIOADDRESS) XGINew_2ndP3C4, 0x06, tempal);

    /* Copy SR21 to 2nd chip */
    tempal = (UCHAR) XGI_GetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x21);
    XGI_SetReg((XGIIOADDRESS) XGINew_2ndP3C4, 0x21, tempal);

    /* Miscellaneous reg(input port 3cch,output port 3c2h) */
    tempal = (UCHAR) XGI_GetRegByte((XGIIOADDRESS) XGINew_P3CC);        /* 3cc */
    XGI_SetRegByte((XGIIOADDRESS) XGINew_2ndP3C2, tempal);

    /* Close key in 2nd chip */
    XGI_SetReg((XGIIOADDRESS) XGINew_2ndP3C4, 0x05, 0x00);
}


/* --------------------------------------------------------------------- */
/* Function : XGI_SetCRT2Group301 */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
BOOLEAN
XGI_SetCRT2Group301(USHORT ModeNo, PXGI_HW_DEVICE_INFO HwDeviceExtension,
                    PVB_DEVICE_INFO pVBInfo)
{
    USHORT tempbx, ModeIdIndex, RefreshRateTableIndex;
    USHORT temp_mode_no;

    tempbx = pVBInfo->VBInfo;
    pVBInfo->SetFlag |= ProgrammingCRT2;

    temp_mode_no = ModeNo;
    XGI_SearchModeID(pVBInfo->SModeIDTable, pVBInfo->EModeIDTable, 0x11,
                     &temp_mode_no, &ModeIdIndex);


    pVBInfo->SelectCRT2Rate = 4;
    RefreshRateTableIndex = XGI_GetRatePtrCRT2(ModeNo, ModeIdIndex, pVBInfo);
    XGI_SaveCRT2Info(ModeNo, pVBInfo);
    XGI_GetCRT2ResInfo(ModeNo, ModeIdIndex, pVBInfo);
    XGI_GetCRT2Data(ModeNo, ModeIdIndex, RefreshRateTableIndex, pVBInfo);
    XGI_PreSetGroup1(ModeNo, ModeIdIndex, HwDeviceExtension,
                     RefreshRateTableIndex, pVBInfo);
    XGI_SetGroup1(ModeNo, ModeIdIndex, HwDeviceExtension,
                  RefreshRateTableIndex, pVBInfo);
    XGI_SetLockRegs(ModeNo, ModeIdIndex, HwDeviceExtension,
                    RefreshRateTableIndex, pVBInfo);
    XGI_SetGroup2(ModeNo, ModeIdIndex, RefreshRateTableIndex,
                  HwDeviceExtension, pVBInfo);
    XGI_SetLCDRegs(ModeNo, ModeIdIndex, HwDeviceExtension,
                   RefreshRateTableIndex, pVBInfo);
    XGI_SetTap4Regs(pVBInfo);
    XGI_SetGroup3(ModeNo, ModeIdIndex, pVBInfo);
    XGI_SetGroup4(ModeNo, ModeIdIndex, RefreshRateTableIndex,
                  HwDeviceExtension, pVBInfo);
    XGI_SetCRT2VCLK(ModeNo, ModeIdIndex, RefreshRateTableIndex, pVBInfo);
    XGI_SetGroup5(ModeNo, ModeIdIndex, pVBInfo);
    XGI_AutoThreshold(pVBInfo);
    return 1;
}


/* --------------------------------------------------------------------- */
/* Function : XGI_AutoThreshold */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_AutoThreshold(PVB_DEVICE_INFO pVBInfo)
{
    if (!(pVBInfo->SetFlag & Win9xDOSMode))
        XGI_SetRegOR((XGIIOADDRESS) pVBInfo->Part1Port, 0x01, 0x40);
}


/* --------------------------------------------------------------------- */
/* Function : XGI_SaveCRT2Info */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_SaveCRT2Info(USHORT ModeNo, PVB_DEVICE_INFO pVBInfo)
{
    USHORT temp1, temp2;

    XGI_SetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x34, ModeNo);     /* reserve CR34 for CRT1 Mode No */
    temp1 = (pVBInfo->VBInfo & SetInSlaveMode) >> 8;
    temp2 = ~(SetInSlaveMode >> 8);
    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->P3d4, 0x31, temp2, temp1);
}


/* --------------------------------------------------------------------- */
/* Function : XGI_GetCRT2ResInfo */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_GetCRT2ResInfo(USHORT ModeNo, USHORT ModeIdIndex, PVB_DEVICE_INFO pVBInfo)
{
    unsigned xres;
    unsigned yres;


    get_mode_xres_yres(ModeNo, ModeIdIndex, pVBInfo, &xres, &yres);

    if ((pVBInfo->VBInfo & SetCRT2ToLCD)
        && !(pVBInfo->LCDInfo & (EnableScalingLCD | LCDNonExpanding))) {
        switch (pVBInfo->LCDResInfo) {
        case Panel1600x1200:
            if (!(pVBInfo->LCDInfo & LCDVESATiming) && (yres == 1024)) {
                yres = 1056;
            }
            break;


        case Panel1280x1024:
            if (yres == 400)
                yres = 405;
            else if (yres == 350)
                yres = 360;
            else if ((pVBInfo->LCDInfo & LCDVESATiming) && (yres == 360)) {
                yres = 375;
            }
            break;


        case Panel1024x768:
            if (!(pVBInfo->LCDInfo & (LCDVESATiming | LCDNonExpanding))) {
                if (yres == 350) {
                    yres = 357;
                }
                else if (yres == 400) {
                    yres = 420;
                }
                else if (yres == 480) {
                    yres = 525;
                }
            }

            break;
        }

        if (xres == 720)
            xres = 640;
    }

    pVBInfo->VGAHDE = xres;
    pVBInfo->HDE = xres;
    pVBInfo->VGAVDE = yres;
    pVBInfo->VDE = yres;
}


/* --------------------------------------------------------------------- */
/* Function : XGI_IsLCDDualLink */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
BOOLEAN
XGI_IsLCDDualLink(PVB_DEVICE_INFO pVBInfo)
{
    return (((pVBInfo->VBInfo & (SetCRT2ToLCD | SetCRT2ToLCDA)) != 0)
            && ((pVBInfo->LCDInfo & SetLCDDualLink) != 0));
}


/* --------------------------------------------------------------------- */
/* Function : XGI_GetCRT2Data */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_GetCRT2Data(USHORT ModeNo, USHORT ModeIdIndex,
                USHORT RefreshRateTableIndex, PVB_DEVICE_INFO pVBInfo)
{
    USHORT tempax = 0, tempbx, modeflag, resinfo;
#ifndef LINUX_XF86
    USHORT CRT2Index, ResIndex;
#endif

    XGI_LCDDataStruct *LCDPtr = NULL;
    XGI_TVDataStruct *TVPtr = NULL;

    if (ModeNo <= 0x13) {
        modeflag = pVBInfo->SModeIDTable[ModeIdIndex].St_ModeFlag;      /* si+St_ResInfo */
        resinfo = pVBInfo->SModeIDTable[ModeIdIndex].St_ResInfo;
    }
    else {
        modeflag = pVBInfo->EModeIDTable[ModeIdIndex].Ext_ModeFlag;     /* si+Ext_ResInfo */
        resinfo = pVBInfo->EModeIDTable[ModeIdIndex].Ext_RESINFO;
    }

    pVBInfo->NewFlickerMode = 0;
    pVBInfo->RVBHRS = 50;

    if (pVBInfo->VBInfo & SetCRT2ToRAMDAC) {
        XGI_GetRAMDAC2DATA(ModeNo, ModeIdIndex, RefreshRateTableIndex,
                           pVBInfo);
        return;
    }

    tempbx = 4;

    if (pVBInfo->VBInfo & (SetCRT2ToLCD | SetCRT2ToLCDA)) {
        LCDPtr =
            (XGI_LCDDataStruct *) XGI_GetLcdPtr(tempbx, ModeNo, ModeIdIndex,
                                                RefreshRateTableIndex,
                                                pVBInfo);

        PDEBUG(ErrorF
               ("C code setmode: ModeNo: 0x%08lX VGAHT:0x%081X  \n", ModeNo,
                LCDPtr->VGAHT));
        pVBInfo->RVBHCMAX = LCDPtr->RVBHCMAX;
        pVBInfo->RVBHCFACT = LCDPtr->RVBHCFACT;
        pVBInfo->VGAHT = LCDPtr->VGAHT;
        pVBInfo->VGAVT = LCDPtr->VGAVT;
        pVBInfo->HT = LCDPtr->LCDHT;
        pVBInfo->VT = LCDPtr->LCDVT;

        if (pVBInfo->LCDResInfo == Panel1024x768) {
            tempax = 1024;
            tempbx = 768;

            if (!(pVBInfo->LCDInfo & LCDVESATiming)) {
                if (pVBInfo->VGAVDE == 357)
                    tempbx = 527;
                else if (pVBInfo->VGAVDE == 420)
                    tempbx = 620;
                else if (pVBInfo->VGAVDE == 525)
                    tempbx = 775;
                else if (pVBInfo->VGAVDE == 600)
                    tempbx = 775;
                /* else if(pVBInfo->VGAVDE==350) tempbx=560; */
                /* else if(pVBInfo->VGAVDE==400) tempbx=640; */
                else
                    tempbx = 768;
            }
            else
                tempbx = 768;
        }
        else if (pVBInfo->LCDResInfo == Panel1024x768x75) {
            tempax = 1024;
            tempbx = 768;
        }
        else if (pVBInfo->LCDResInfo == Panel1280x1024) {
            tempax = 1280;
            if (pVBInfo->VGAVDE == 360)
                tempbx = 768;
            else if (pVBInfo->VGAVDE == 375)
                tempbx = 800;
            else if (pVBInfo->VGAVDE == 405)
                tempbx = 864;
            else
                tempbx = 1024;
        }
        else if (pVBInfo->LCDResInfo == Panel1280x1024x75) {
            tempax = 1280;
            tempbx = 1024;
        }
        else if (pVBInfo->LCDResInfo == Panel1280x960) {
            tempax = 1280;
            if (pVBInfo->VGAVDE == 350)
                tempbx = 700;
            else if (pVBInfo->VGAVDE == 400)
                tempbx = 800;
            else if (pVBInfo->VGAVDE == 1024)
                tempbx = 960;
            else
                tempbx = 960;
        }
        else if (pVBInfo->LCDResInfo == Panel1400x1050) {
            tempax = 1400;
            tempbx = 1050;

            if (pVBInfo->VGAVDE == 1024) {
                tempax = 1280;
                tempbx = 1024;
            }
        }
        else if (pVBInfo->LCDResInfo == Panel1600x1200) {
            tempax = 1600;
            tempbx = 1200;      /* alan 10/14/2003 */
            if (!(pVBInfo->LCDInfo & LCDVESATiming)) {
                if (pVBInfo->VGAVDE == 350)
                    tempbx = 875;
                else if (pVBInfo->VGAVDE == 400)
                    tempbx = 1000;
            }
        }

        if (pVBInfo->LCDInfo & (LCDNonExpanding | EnableScalingLCD)) {
            tempax = pVBInfo->VGAHDE;
            tempbx = pVBInfo->VGAVDE;
        }

        pVBInfo->HDE = tempax;
        pVBInfo->VDE = tempbx;
        return;
    }

    if (pVBInfo->VBInfo & (SetCRT2ToTV)) {
        tempbx = 4;
        TVPtr =
            (XGI_TVDataStruct *) XGI_GetTVPtr(tempbx, ModeNo, ModeIdIndex,
                                              RefreshRateTableIndex, pVBInfo);
        pVBInfo->RVBHCMAX = TVPtr->RVBHCMAX;
        pVBInfo->RVBHCFACT = TVPtr->RVBHCFACT;
        pVBInfo->VGAHT = TVPtr->VGAHT;
        pVBInfo->VGAVT = TVPtr->VGAVT;
        pVBInfo->HDE = TVPtr->TVHDE;
        pVBInfo->VDE = TVPtr->TVVDE;
        pVBInfo->RVBHRS = TVPtr->RVBHRS;
        pVBInfo->NewFlickerMode = TVPtr->FlickerMode;

        if (pVBInfo->VBInfo & SetCRT2ToHiVisionTV) {
            if (resinfo == 0x08)
                pVBInfo->NewFlickerMode = 0x40;
            else if (resinfo == 0x09)
                pVBInfo->NewFlickerMode = 0x40;
            else if (resinfo == 0x12)
                pVBInfo->NewFlickerMode = 0x40;

            if (pVBInfo->VGAVDE == 350)
                pVBInfo->TVInfo |= TVSimuMode;

            tempax = ExtHiTVHT;
            tempbx = ExtHiTVVT;

            if (pVBInfo->VBInfo & SetInSlaveMode) {
                if (pVBInfo->TVInfo & TVSimuMode) {
                    tempax = StHiTVHT;
                    tempbx = StHiTVVT;

                    if (!(modeflag & Charx8Dot)) {
                        tempax = StHiTextTVHT;
                        tempbx = StHiTextTVVT;
                    }
                }
            }
        }
        else if (pVBInfo->VBInfo & SetCRT2ToYPbPr) {
            if (pVBInfo->TVInfo & SetYPbPrMode750p) {
                tempax = YPbPrTV750pHT; /* Ext750pTVHT */
                tempbx = YPbPrTV750pVT; /* Ext750pTVVT */
            }

            if (pVBInfo->TVInfo & SetYPbPrMode525p) {
                tempax = YPbPrTV525pHT; /* Ext525pTVHT */
                tempbx = YPbPrTV525pVT; /* Ext525pTVVT */
            }
            else if (pVBInfo->TVInfo & SetYPbPrMode525i) {
                tempax = YPbPrTV525iHT; /* Ext525iTVHT */
                tempbx = YPbPrTV525iVT; /* Ext525iTVVT */
                if (pVBInfo->TVInfo & NTSC1024x768)
                    tempax = NTSC1024x768HT;
            }
        }
        else {
            tempax = PALHT;
            tempbx = PALVT;
            if (!(pVBInfo->TVInfo & SetPALTV)) {
                tempax = NTSCHT;
                tempbx = NTSCVT;
                if (pVBInfo->TVInfo & NTSC1024x768)
                    tempax = NTSC1024x768HT;
            }
        }

        pVBInfo->HT = tempax;
        pVBInfo->VT = tempbx;
        return;
    }
}


/* --------------------------------------------------------------------- */
/* Function : XGI_SetCRT2VCLK */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_SetCRT2VCLK(USHORT ModeNo, USHORT ModeIdIndex,
                USHORT RefreshRateTableIndex, PVB_DEVICE_INFO pVBInfo)
{
    UCHAR di[2];
    const unsigned vclkindex =
        XGI_GetVCLKPtr(RefreshRateTableIndex, ModeNo, ModeIdIndex, pVBInfo);

    XGI_GetVCLKLen(vclkindex, di, pVBInfo);
    XGI_GetLCDVCLKPtr(di, pVBInfo);

    if (pVBInfo->VBType & VB_XGI301) {  /* shampoo 0129 *//* 301 */
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part4Port, 0x0A, 0x10);
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part4Port, 0x0B, di[1]);
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part4Port, 0x0A, di[0]);
    }
    else {                      /* 301b/302b/301lv/302lv */
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part4Port, 0x0A, di[0]);
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part4Port, 0x0B, di[1]);
    }

    if ((pVBInfo->LCDInfo & EnableReduceTiming)
        && (pVBInfo->LCDResInfo == Panel1600x1200)) {
        if (pVBInfo->EModeIDTable[ModeIdIndex].Ext_RESINFO == 0x0A) {
            XGI_SetReg((XGIIOADDRESS) pVBInfo->Part4Port, 0x0A, 0x5A);
            XGI_SetReg((XGIIOADDRESS) pVBInfo->Part4Port, 0x0B, 0x24);
        }
    }

    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part4Port, 0x00, 0x12);

    if (pVBInfo->VBInfo & SetCRT2ToRAMDAC)
        XGI_SetRegOR((XGIIOADDRESS) pVBInfo->Part4Port, 0x12, 0x28);
    else
        XGI_SetRegOR((XGIIOADDRESS) pVBInfo->Part4Port, 0x12, 0x08);
}


/* --------------------------------------------------------------------- */
/* Function : XGI_GETLCDVCLKPtr */
/* Input : */
/* Output : al -> VCLK Index */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_GetLCDVCLKPtr(UCHAR *di, PVB_DEVICE_INFO pVBInfo)
{
    if (pVBInfo->VBInfo & (SetCRT2ToLCD | SetCRT2ToLCDA)) {
        if ((pVBInfo->IF_DEF_ScaleLCD != 1) 
            || !(pVBInfo->LCDInfo & EnableScalingLCD)) {
            const unsigned index = XGI_GetLCDCapPtr1(pVBInfo);

            if (pVBInfo->VBInfo & SetCRT2ToLCD) {   /* LCDB */
                di[0] = pVBInfo->LCDCapList[index].LCUCHAR_VCLKData1;
                di[1] = pVBInfo->LCDCapList[index].LCUCHAR_VCLKData2;
            }
            else {                  /* LCDA */
                di[0] = pVBInfo->LCDCapList[index].LCDA_VCLKData1;
                di[1] = pVBInfo->LCDCapList[index].LCDA_VCLKData2;
            }
        }
    }

    return;
}


/* --------------------------------------------------------------------- */
/* Function : XGI_GetVCLKPtr */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
unsigned
XGI_GetVCLKPtr(USHORT RefreshRateTableIndex, USHORT ModeNo,
               USHORT ModeIdIndex, PVB_DEVICE_INFO pVBInfo)
{
    unsigned vclk;
    const unsigned modeflag = (ModeNo <= 0x13)
        ? pVBInfo->SModeIDTable[ModeIdIndex].St_ModeFlag
        : pVBInfo->EModeIDTable[ModeIdIndex].Ext_ModeFlag;


    if ((pVBInfo->SetFlag & ProgrammingCRT2) 
        && (!(pVBInfo->LCDInfo & EnableScalingLCD))) {     /* {LCDA/LCDB} */
        const unsigned  index = XGI_GetLCDCapPtr(pVBInfo);
        vclk = pVBInfo->LCDCapList[index].LCD_VCLK;

        if (pVBInfo->VBInfo & (SetCRT2ToLCD | SetCRT2ToLCDA))
            return vclk;

        /* {TV} */
        if (pVBInfo->
            VBType & (VB_XGI301B | VB_XGI302B | VB_XGI301LV | VB_XGI302LV |
                      VB_XGI301C)) {
            if (pVBInfo->VBInfo & SetCRT2ToHiVisionTV) {
                if (pVBInfo->TVInfo & TVSimuMode) {
                    vclk = (modeflag & Charx8Dot)
                        ? HiTVSimuVCLK : HiTVTextVCLK;
                }
                else {
                    vclk = (pVBInfo->TVInfo & RPLLDIV2XO)
                        ? HiTVVCLKDIV2 : HiTVVCLK;
                }

                return vclk;
            }
            else if (pVBInfo->TVInfo & SetYPbPrMode750p) {
                return YPbPr750pVCLK;
            }
            else if (pVBInfo->TVInfo & SetYPbPrMode525p) {
                return YPbPr525pVCLK;
            }

            vclk = NTSC1024VCLK;

            if (!(pVBInfo->TVInfo & NTSC1024x768)) {
                vclk = (pVBInfo->TVInfo & RPLLDIV2XO)
                    ? TVVCLKDIV2 : TVVCLK;
            }

            if (pVBInfo->VBInfo & SetCRT2ToTV)
                return vclk;
        }
    }                           /* {End of VB} */

    vclk = XGI_GetRegByte((XGIIOADDRESS) (pVBInfo->P3ca + 0x02));
    vclk = (vclk >> 2) & 0x03;

    /* for Dot8 Scaling LCD */
    if ((pVBInfo->LCDInfo & EnableScalingLCD)
        && (modeflag & Charx8Dot) 
        && ((pVBInfo->IF_DEF_VideoCapture) == 1)) {
        vclk = VCLK25_175;       /* ; set to VCLK25MHz always */
    }

    if (ModeNo <= 0x13)
        return vclk;

    return pVBInfo->RefIndex[RefreshRateTableIndex].Ext_CRTVCLK;
}


/* --------------------------------------------------------------------- */
/* Function : XGI_GetVCLKLen */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_GetVCLKLen(unsigned vclkindex, UCHAR *di, PVB_DEVICE_INFO pVBInfo)
{
    if (pVBInfo->
        VBType & (VB_XGI301 | VB_XGI301B | VB_XGI302B | VB_XGI301LV |
                  VB_XGI302LV | VB_XGI301C)) {
        if ((!(pVBInfo->VBInfo & SetCRT2ToLCDA))
            && (pVBInfo->SetFlag & ProgrammingCRT2)) {
            di[0] = XGI_VBVCLKData[vclkindex].SR2B;
            di[1] = XGI_VBVCLKData[vclkindex].SR2C;
        }
    }
    else {
        di[0] = XGI_VCLKData[vclkindex].SR2B;
        di[1] = XGI_VCLKData[vclkindex].SR2C;
    }
}


/* --------------------------------------------------------------------- */
/* Function : XGI_SetCRT2Offset */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_SetCRT2Offset(USHORT ModeNo,
                  USHORT ModeIdIndex, USHORT RefreshRateTableIndex,
                  PXGI_HW_DEVICE_INFO HwDeviceExtension,
                  PVB_DEVICE_INFO pVBInfo)
{
    USHORT offset;
    UCHAR temp;

    if (pVBInfo->VBInfo & SetInSlaveMode) {
        return;
    }

    offset =
        XGI_GetOffset(ModeNo, ModeIdIndex, RefreshRateTableIndex,
                      HwDeviceExtension, pVBInfo);
    temp = (UCHAR) (offset & 0xFF);
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x07, temp);
    temp = (UCHAR) ((offset & 0xFF00) >> 8);
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x09, temp);
    temp = (UCHAR) (((offset >> 3) & 0xFF) + 1);
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x03, temp);
}


/* --------------------------------------------------------------------- */
/* Function : XGI_GetOffset */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
USHORT
XGI_GetOffset(USHORT ModeNo, USHORT ModeIdIndex, USHORT RefreshRateTableIndex,
              PXGI_HW_DEVICE_INFO HwDeviceExtension, PVB_DEVICE_INFO pVBInfo)
{
    USHORT temp,
        colordepth,
        modeinfo, index, infoflag;

    modeinfo = pVBInfo->EModeIDTable[ModeIdIndex].Ext_ModeInfo;
    if (ModeNo <= 0x14)
        infoflag = 0;
    else
        infoflag = pVBInfo->RefIndex[RefreshRateTableIndex].Ext_InfoFlag;


    index = (modeinfo >> 8) & 0xFF;

    temp = pVBInfo->ScreenOffset[index];

    if (infoflag & InterlaceMode) {
        temp = temp << 1;
    }

    colordepth = XGI_GetColorDepth(ModeNo, ModeIdIndex, pVBInfo);

    if ((ModeNo >= 0x26) && (ModeNo <= 0x28)) {
        return (temp * colordepth + (colordepth >> 1));
    }
    else
        return (temp * colordepth);
}


/* --------------------------------------------------------------------- */
/* Function : XGI_SetCRT2FIFO */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_SetCRT2FIFO(PVB_DEVICE_INFO pVBInfo)
{
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x01, 0x3B);  /* threshold high ,disable auto threshold */
    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part1Port, 0x02, ~(0x3F), 0x04);    /* threshold low default 04h */
}


/* --------------------------------------------------------------------- */
/* Function : XGI_PreSetGroup1 */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_PreSetGroup1(USHORT ModeNo, USHORT ModeIdIndex,
                 PXGI_HW_DEVICE_INFO HwDeviceExtension,
                 USHORT RefreshRateTableIndex, PVB_DEVICE_INFO pVBInfo)
{
    USHORT tempcx = 0, CRT1Index = 0, resinfo = 0;
#ifndef LINUX_XF86
    USHORT temp = 0, tempax = 0, tempbx = 0, pushbx = 0, modeflag;
#endif

    if (ModeNo > 0x13) {
        CRT1Index = pVBInfo->RefIndex[RefreshRateTableIndex].Ext_CRT1CRTC;
        CRT1Index &= IndexMask;
        resinfo = pVBInfo->EModeIDTable[ModeIdIndex].Ext_RESINFO;
    }

    XGI_SetCRT2Offset(ModeNo, ModeIdIndex, RefreshRateTableIndex,
                      HwDeviceExtension, pVBInfo);
    XGI_SetCRT2FIFO(pVBInfo);
    /* XGI_SetCRT2Sync(ModeNo,RefreshRateTableIndex); */

    for (tempcx = 4; tempcx < 7; tempcx++) {
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, tempcx, 0x0);
    }

    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x02, 0x44);  /* temp 0206 */
}


/* --------------------------------------------------------------------- */
/* Function : XGI_SetGroup1 */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_SetGroup1(USHORT ModeNo, USHORT ModeIdIndex,
              PXGI_HW_DEVICE_INFO HwDeviceExtension,
              USHORT RefreshRateTableIndex, PVB_DEVICE_INFO pVBInfo)
{
    USHORT temp = 0,
        tempax = 0,
        tempbx = 0,
        tempcx = 0, pushbx = 0, CRT1Index = 0, modeflag, resinfo = 0;

    if (ModeNo > 0x13) {
        CRT1Index = pVBInfo->RefIndex[RefreshRateTableIndex].Ext_CRT1CRTC;
        CRT1Index &= IndexMask;
        resinfo = pVBInfo->EModeIDTable[ModeIdIndex].Ext_RESINFO;
    }

    if (ModeNo <= 0x13) {
        modeflag = pVBInfo->SModeIDTable[ModeIdIndex].St_ModeFlag;
    }
    else {
        modeflag = pVBInfo->EModeIDTable[ModeIdIndex].Ext_ModeFlag;
    }

    /* bainy change table name */
    if (modeflag & HalfDCLK) {
        temp = (pVBInfo->VGAHT / 2 - 1) & 0x0FF;        /* BTVGA2HT 0x08,0x09 */
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x08, temp);
        temp = (((pVBInfo->VGAHT / 2 - 1) & 0xFF00) >> 8) << 4;
        XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part1Port, 0x09, ~0x0F0,
                        temp);
        temp = (pVBInfo->VGAHDE / 2 + 16) & 0x0FF;      /* BTVGA2HDEE 0x0A,0x0C */
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x0A, temp);
        tempcx = ((pVBInfo->VGAHT - pVBInfo->VGAHDE) / 2) >> 2;
        pushbx = pVBInfo->VGAHDE / 2 + 16;
        tempcx = tempcx >> 1;
        tempbx = pushbx + tempcx;       /* bx BTVGA@HRS 0x0B,0x0C */
        tempcx += tempbx;

        if (pVBInfo->VBInfo & SetCRT2ToRAMDAC) {
            tempbx = pVBInfo->XGINEWUB_CRT1Table[CRT1Index].CR[4];
            tempbx |=
                ((pVBInfo->XGINEWUB_CRT1Table[CRT1Index].CR[14] & 0xC0) << 2);
            tempbx = (tempbx - 3) << 3; /* (VGAHRS-3)*8 */
            tempcx = pVBInfo->XGINEWUB_CRT1Table[CRT1Index].CR[5];
            tempcx &= 0x1F;
            temp = pVBInfo->XGINEWUB_CRT1Table[CRT1Index].CR[15];
            temp = (temp & 0x04) << (5 - 2);    /* VGAHRE D[5] */
            tempcx = ((tempcx | temp) - 3) << 3;        /* (VGAHRE-3)*8 */
        }

        tempbx += 4;
        tempcx += 4;

        if (tempcx > (pVBInfo->VGAHT / 2))
            tempcx = pVBInfo->VGAHT / 2;

        temp = tempbx & 0x00FF;

        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x0B, temp);
    }
    else {
        temp = (pVBInfo->VGAHT - 1) & 0x0FF;    /* BTVGA2HT 0x08,0x09 */
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x08, temp);
        temp = (((pVBInfo->VGAHT - 1) & 0xFF00) >> 8) << 4;
        XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part1Port, 0x09, ~0x0F0,
                        temp);
        temp = (pVBInfo->VGAHDE + 16) & 0x0FF;  /* BTVGA2HDEE 0x0A,0x0C */
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x0A, temp);
        tempcx = (pVBInfo->VGAHT - pVBInfo->VGAHDE) >> 2;       /* cx */
        pushbx = pVBInfo->VGAHDE + 16;
        tempcx = tempcx >> 1;
        tempbx = pushbx + tempcx;       /* bx BTVGA@HRS 0x0B,0x0C */
        tempcx += tempbx;

        if (pVBInfo->VBInfo & SetCRT2ToRAMDAC) {
            tempbx = pVBInfo->XGINEWUB_CRT1Table[CRT1Index].CR[3];
            tempbx |=
                ((pVBInfo->XGINEWUB_CRT1Table[CRT1Index].CR[5] & 0xC0) << 2);
            tempbx = (tempbx - 3) << 3; /* (VGAHRS-3)*8 */
            tempcx = pVBInfo->XGINEWUB_CRT1Table[CRT1Index].CR[4];
            tempcx &= 0x1F;
            temp = pVBInfo->XGINEWUB_CRT1Table[CRT1Index].CR[6];
            temp = (temp & 0x04) << (5 - 2);    /* VGAHRE D[5] */
            tempcx = ((tempcx | temp) - 3) << 3;        /* (VGAHRE-3)*8 */
            tempbx += 16;
            tempcx += 16;
        }

        if (tempcx > pVBInfo->VGAHT)
            tempcx = pVBInfo->VGAHT;

        temp = tempbx & 0x00FF;
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x0B, temp);
    }

    tempax = (tempax & 0x00FF) | (tempbx & 0xFF00);
    tempbx = pushbx;
    tempbx = (tempbx & 0x00FF) | ((tempbx & 0xFF00) << 4);
    tempax |= (tempbx & 0xFF00);
    temp = (tempax & 0xFF00) >> 8;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x0C, temp);
    temp = tempcx & 0x00FF;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x0D, temp);
    tempcx = (pVBInfo->VGAVT - 1);
    temp = tempcx & 0x00FF;

    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x0E, temp);
    tempbx = pVBInfo->VGAVDE - 1;
    temp = tempbx & 0x00FF;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x0F, temp);
    temp = ((tempbx & 0xFF00) << 3) >> 8;
    temp |= ((tempcx & 0xFF00) >> 8);
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x12, temp);

    tempax = pVBInfo->VGAVDE;
    tempbx = pVBInfo->VGAVDE;
    tempcx = pVBInfo->VGAVT;
    tempbx = (pVBInfo->VGAVT + pVBInfo->VGAVDE) >> 1;   /* BTVGA2VRS 0x10,0x11 */
    tempcx = ((pVBInfo->VGAVT - pVBInfo->VGAVDE) >> 4) + tempbx + 1;    /* BTVGA2VRE 0x11 */

    if (pVBInfo->VBInfo & SetCRT2ToRAMDAC) {
        tempbx = pVBInfo->XGINEWUB_CRT1Table[CRT1Index].CR[10];
        temp = pVBInfo->XGINEWUB_CRT1Table[CRT1Index].CR[9];

        if (temp & 0x04)
            tempbx |= 0x0100;

        if (temp & 0x080)
            tempbx |= 0x0200;

        temp = pVBInfo->XGINEWUB_CRT1Table[CRT1Index].CR[14];

        if (temp & 0x08)
            tempbx |= 0x0400;

        temp = pVBInfo->XGINEWUB_CRT1Table[CRT1Index].CR[11];
        tempcx = (tempcx & 0xFF00) | (temp & 0x00FF);
    }

    temp = tempbx & 0x00FF;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x10, temp);
    temp = ((tempbx & 0xFF00) >> 8) << 4;
    temp = ((tempcx & 0x000F) | (temp));
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x11, temp);
    tempax = 0;

    if (modeflag & DoubleScanMode)
        tempax |= 0x80;

    if (modeflag & HalfDCLK)
        tempax |= 0x40;

    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part1Port, 0x2C, ~0x0C0, tempax);
}


/* --------------------------------------------------------------------- */
/* Function : XGI_SetLockRegs */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_SetLockRegs(USHORT ModeNo, USHORT ModeIdIndex,
                PXGI_HW_DEVICE_INFO HwDeviceExtension,
                USHORT RefreshRateTableIndex, PVB_DEVICE_INFO pVBInfo)
{
    USHORT push1,
        push2, tempax, tempbx = 0, tempcx, temp, resinfo, modeflag, CRT1Index;

    if (ModeNo <= 0x13) {
        modeflag = pVBInfo->SModeIDTable[ModeIdIndex].St_ModeFlag;      /* si+St_ResInfo */
        resinfo = pVBInfo->SModeIDTable[ModeIdIndex].St_ResInfo;
    }
    else {
        modeflag = pVBInfo->EModeIDTable[ModeIdIndex].Ext_ModeFlag;     /* si+Ext_ResInfo */
        resinfo = pVBInfo->EModeIDTable[ModeIdIndex].Ext_RESINFO;
        CRT1Index = pVBInfo->RefIndex[RefreshRateTableIndex].Ext_CRT1CRTC;
        CRT1Index &= IndexMask;
    }

    if (!(pVBInfo->VBInfo & SetInSlaveMode)) {
        return;
    }

    temp = 0xFF;                /* set MAX HT */
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x03, temp);
    /* if ( modeflag & Charx8Dot ) tempcx = 0x08 ; */
    /* else */
    tempcx = 0x08;

    if (pVBInfo->VBType & (VB_XGI301LV | VB_XGI302LV | VB_XGI301C))
        modeflag |= Charx8Dot;

    tempax = pVBInfo->VGAHDE;   /* 0x04 Horizontal Display End */

    if (modeflag & HalfDCLK)
        tempax = tempax >> 1;

    tempax = (tempax / tempcx) - 1;
    tempbx |= ((tempax & 0x00FF) << 8);
    temp = tempax & 0x00FF;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x04, temp);

    temp = (tempbx & 0xFF00) >> 8;

    if (pVBInfo->VBInfo & SetCRT2ToTV) {
        if (!
            (pVBInfo->
             VBType & (VB_XGI301B | VB_XGI302B | VB_XGI301LV | VB_XGI302LV |
                       VB_XGI301C)))
            temp += 2;

        if (pVBInfo->VBInfo & SetCRT2ToHiVisionTV) {
            if (pVBInfo->VBType & VB_XGI301LV) {
                if (pVBInfo->VBExtInfo == VB_YPbPr1080i) {
                    if (resinfo == 7)
                        temp -= 2;
                }
            }
            else if (resinfo == 7)
                temp -= 2;
        }
    }

    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x05, temp);  /* 0x05 Horizontal Display Start */
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x06, 0x03);  /* 0x06 Horizontal Blank end */

    if (!(pVBInfo->VBInfo & DisableCRT2Display)) {      /* 030226 bainy */
        if (pVBInfo->VBInfo & SetCRT2ToTV)
            tempax = pVBInfo->VGAHT;
        else
            tempax = XGI_GetVGAHT2(pVBInfo);
    }

    if (tempax >= pVBInfo->VGAHT) {
        tempax = pVBInfo->VGAHT;
    }

    if (modeflag & HalfDCLK) {
        tempax = tempax >> 1;
    }

    tempax = (tempax / tempcx) - 5;
    tempcx = tempax;            /* 20030401 0x07 horizontal Retrace Start */
    if (pVBInfo->VBInfo & SetCRT2ToHiVisionTV) {
        temp = (tempbx & 0x00FF) - 1;
        if (!(modeflag & HalfDCLK)) {
            temp -= 6;
            if (pVBInfo->TVInfo & TVSimuMode) {
                temp -= 4;
                if (ModeNo > 0x13)
                    temp -= 10;
            }
        }
    }
    else {
        /* tempcx = tempbx & 0x00FF ; */
        tempbx = (tempbx & 0xFF00) >> 8;
        tempcx = (tempcx + tempbx) >> 1;
        temp = (tempcx & 0x00FF) + 2;

        if (pVBInfo->VBInfo & SetCRT2ToTV) {
            temp -= 1;
            if (!(modeflag & HalfDCLK)) {
                if ((modeflag & Charx8Dot)) {
                    temp += 4;
                    if (pVBInfo->VGAHDE >= 800) {
                        temp -= 6;
                    }
                }
            }
        }
        else {
            if (!(modeflag & HalfDCLK)) {
                temp -= 4;
                if (pVBInfo->LCDResInfo != Panel1280x960) {
                    if (pVBInfo->VGAHDE >= 800) {
                        temp -= 7;
                        if (pVBInfo->ModeType == ModeEGA) {
                            if (pVBInfo->VGAVDE == 1024) {
                                temp += 15;
                                if (pVBInfo->LCDResInfo != Panel1280x1024) {
                                    temp += 7;
                                }
                            }
                        }

                        if (pVBInfo->VGAHDE >= 1280) {
                            if (pVBInfo->LCDResInfo != Panel1280x960) {
                                if (!
                                    (pVBInfo->
                                     LCDInfo & (LCDNonExpanding |
                                                EnableScalingLCD))) {
                                    temp += 28;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x07, temp);  /* 0x07 Horizontal Retrace Start */
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x08, 0);     /* 0x08 Horizontal Retrace End */

    if (pVBInfo->VBInfo & SetCRT2ToTV) {
        if (pVBInfo->TVInfo & TVSimuMode) {
            if ((ModeNo == 0x06) || (ModeNo == 0x10) || (ModeNo == 0x11)
                || (ModeNo == 0x13) || (ModeNo == 0x0F)) {
                XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x07, 0x5b);
                XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x08, 0x03);
            }

            if ((ModeNo == 0x00) || (ModeNo == 0x01)) {
                if (pVBInfo->TVInfo & SetNTSCTV) {
                    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x07, 0x2A);
                    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x08, 0x61);
                }
                else {
                    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x07, 0x2A);
                    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x08, 0x41);
                    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x0C, 0xF0);
                }
            }

            if ((ModeNo == 0x02) || (ModeNo == 0x03) || (ModeNo == 0x07)) {
                if (pVBInfo->TVInfo & SetNTSCTV) {
                    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x07, 0x54);
                    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x08, 0x00);
                }
                else {
                    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x07, 0x55);
                    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x08, 0x00);
                    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x0C, 0xF0);
                }
            }

            if ((ModeNo == 0x04) || (ModeNo == 0x05) || (ModeNo == 0x0D)
                || (ModeNo == 0x50)) {
                if (pVBInfo->TVInfo & SetNTSCTV) {
                    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x07, 0x30);
                    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x08, 0x03);
                }
                else {
                    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x07, 0x2f);
                    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x08, 0x02);
                }
            }
        }
    }

    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x18, 0x03);  /* 0x18 SR0B */
    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part1Port, 0x19, 0xF0, 0x00);
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x09, 0xFF);  /* 0x09 Set Max VT */

    tempbx = pVBInfo->VGAVT;
    push1 = tempbx;
    tempcx = 0x121;
    tempbx = pVBInfo->VGAVDE;   /* 0x0E Virtical Display End */

    if (tempbx == 357)
        tempbx = 350;
    if (tempbx == 360)
        tempbx = 350;
    if (tempbx == 375)
        tempbx = 350;
    if (tempbx == 405)
        tempbx = 400;
    if (tempbx == 525)
        tempbx = 480;

    push2 = tempbx;

    if (pVBInfo->VBInfo & SetCRT2ToLCD) {
        if (pVBInfo->LCDResInfo == Panel1024x768) {
            if (!(pVBInfo->LCDInfo & LCDVESATiming)) {
                if (tempbx == 350)
                    tempbx += 5;
                if (tempbx == 480)
                    tempbx += 5;
            }
        }
    }
    tempbx--;
    temp = tempbx & 0x00FF;
    tempbx--;
    temp = tempbx & 0x00FF;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x10, temp);  /* 0x10 vertical Blank Start */
    tempbx = push2;
    tempbx--;
    temp = tempbx & 0x00FF;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x0E, temp);

    if (tempbx & 0x0100) {
        tempcx |= 0x0002;
    }

    tempax = 0x000B;

    if (modeflag & DoubleScanMode) {
        tempax |= 0x08000;
    }

    if (tempbx & 0x0200) {
        tempcx |= 0x0040;
    }

    temp = (tempax & 0xFF00) >> 8;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x0B, temp);

    if (tempbx & 0x0400) {
        tempcx |= 0x0600;
    }

    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x11, 0x00);  /* 0x11 Vertival Blank End */

    tempax = push1;
    tempax -= tempbx;           /* 0x0C Vertical Retrace Start */
    tempax = tempax >> 2;
    push1 = tempax;             /* push ax */

    if (resinfo != 0x09) {
        tempax = tempax << 1;
        tempbx += tempax;
    }

    if (pVBInfo->VBInfo & SetCRT2ToHiVisionTV) {
        if (pVBInfo->VBType & VB_XGI301LV) {
            if (pVBInfo->TVInfo & SetYPbPrMode1080i)
                tempbx -= 10;
            else {
                if (pVBInfo->TVInfo & TVSimuMode) {
                    if (pVBInfo->TVInfo & SetPALTV) {
                        if (pVBInfo->VBType & VB_XGI301LV) {
                            if (!
                                (pVBInfo->
                                 TVInfo & (SetYPbPrMode525p | SetYPbPrMode750p
                                           | SetYPbPrMode1080i)))
                                tempbx += 40;
                        }
                        else
                            tempbx += 40;
                    }
                }
            }
        }
        else
            tempbx -= 10;
    }
    else {
        if (pVBInfo->TVInfo & TVSimuMode) {
            if (pVBInfo->TVInfo & SetPALTV) {
                if (pVBInfo->VBType & VB_XGI301LV) {
                    if (!
                        (pVBInfo->
                         TVInfo & (SetYPbPrMode525p | SetYPbPrMode750p |
                                   SetYPbPrMode1080i)))
                        tempbx += 40;
                }
                else
                    tempbx += 40;
            }
        }
    }
    tempax = push1;
    tempax = tempax >> 2;
    tempax++;
    tempax += tempbx;
    push1 = tempax;             /* push ax */

    if ((pVBInfo->TVInfo & SetPALTV)) {
        if (tempbx <= 513) {
            if (tempax >= 513) {
                tempbx = 513;
            }
        }
    }

    temp = tempbx & 0x00FF;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x0C, temp);
    tempbx--;
    temp = tempbx & 0x00FF;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x10, temp);

    if (tempbx & 0x0100) {
        tempcx |= 0x0008;
    }

    if (tempbx & 0x0200) {
        XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part1Port, 0x0B, 0x0FF, 0x20);
    }

    tempbx++;

    if (tempbx & 0x0100) {
        tempcx |= 0x0004;
    }

    if (tempbx & 0x0200) {
        tempcx |= 0x0080;
    }

    if (tempbx & 0x0400) {
        tempcx |= 0x0C00;
    }

    tempbx = push1;             /* pop ax */
    temp = tempbx & 0x00FF;
    temp &= 0x0F;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x0D, temp);  /* 0x0D vertical Retrace End */

    if (tempbx & 0x0010) {
        tempcx |= 0x2000;
    }

    temp = tempcx & 0x00FF;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x0A, temp);  /* 0x0A CR07 */
    temp = (tempcx & 0x0FF00) >> 8;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x17, temp);  /* 0x17 SR0A */
    tempax = modeflag;
    temp = (tempax & 0xFF00) >> 8;

    temp = (temp >> 1) & 0x09;

    if (pVBInfo->VBType & (VB_XGI301LV | VB_XGI302LV | VB_XGI301C))
        temp |= 0x01;

    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x16, temp);  /* 0x16 SR01 */
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x0F, 0);     /* 0x0F CR14 */
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x12, 0);     /* 0x12 CR17 */

    if (pVBInfo->LCDInfo & LCDRGB18Bit)
        temp = 0x80;
    else
        temp = 0x00;

    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x1A, temp);  /* 0x1A SR0E */

    return;
}


/* --------------------------------------------------------------------- */
/* Function : XGI_SetGroup2 */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_SetGroup2(USHORT ModeNo, USHORT ModeIdIndex, USHORT RefreshRateTableIndex,
              PXGI_HW_DEVICE_INFO HwDeviceExtension, PVB_DEVICE_INFO pVBInfo)
{
    USHORT i,
        j,
        tempax,
        tempbx, tempcx, temp, push1, push2, modeflag, resinfo, crt2crtc;
#ifndef LINUX_XF86
    USHORT temp1, temp3, resindex, xres;
#endif
/*           XGINew_RY1COE = 0 ,
           XGINew_RY2COE = 0 ,
           XGINew_RY3COE = 0 ,
           XGINew_RY4COE = 0 ,
           XGINew_RY5COE = 0 ,
           XGINew_RY6COE = 0 ,
           XGINew_RY7COE = 0 ;
*/

#ifndef LINUX_XF86
    UCHAR *PhasePoint;
#endif
    const UCHAR *TimingPoint;

    ULONG longtemp, tempeax, tempebx, temp2, tempecx;

    if (ModeNo <= 0x13) {
        modeflag = pVBInfo->SModeIDTable[ModeIdIndex].St_ModeFlag;      /* si+St_ResInfo */
        resinfo = pVBInfo->SModeIDTable[ModeIdIndex].St_ResInfo;
        crt2crtc = pVBInfo->SModeIDTable[ModeIdIndex].St_CRT2CRTC;
    }
    else {
        modeflag = pVBInfo->EModeIDTable[ModeIdIndex].Ext_ModeFlag;     /* si+Ext_ResInfo */
        resinfo = pVBInfo->EModeIDTable[ModeIdIndex].Ext_RESINFO;
        crt2crtc = pVBInfo->RefIndex[RefreshRateTableIndex].Ext_CRT2CRTC;
    }

    tempax = 0;

    if (!(pVBInfo->VBInfo & SetCRT2ToAVIDEO))
        tempax |= 0x0800;

    if (!(pVBInfo->VBInfo & SetCRT2ToSVIDEO))
        tempax |= 0x0400;

    if (pVBInfo->VBInfo & SetCRT2ToSCART)
        tempax |= 0x0200;

    if (!(pVBInfo->TVInfo & SetPALTV))
        tempax |= 0x1000;

    if (pVBInfo->VBInfo & SetCRT2ToHiVisionTV)
        tempax |= 0x0100;

    if (pVBInfo->TVInfo & (SetYPbPrMode525p | SetYPbPrMode750p))
        tempax &= 0xfe00;

    ErrorF("Part2 0 = %x ",
           XGI_GetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x0));
    ErrorF(" pVBInfo->VBInfo =%x", pVBInfo->VBInfo);

    tempax = (tempax & 0xff00) >> 8;
    ErrorF("tempax = %x ", tempax);
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x0, tempax);
    TimingPoint = pVBInfo->NTSCTiming;

    if (pVBInfo->TVInfo & SetPALTV) {
        TimingPoint = pVBInfo->PALTiming;
    }

    if (pVBInfo->VBInfo & SetCRT2ToHiVisionTV) {
        TimingPoint = pVBInfo->HiTVExtTiming;

        if (pVBInfo->VBInfo & SetInSlaveMode)
            TimingPoint = pVBInfo->HiTVSt2Timing;

        if (pVBInfo->SetFlag & TVSimuMode)
            TimingPoint = pVBInfo->HiTVSt1Timing;

        if (!(modeflag & Charx8Dot))
            TimingPoint = pVBInfo->HiTVTextTiming;
    }

    if (pVBInfo->VBInfo & SetCRT2ToYPbPr) {
        if (pVBInfo->TVInfo & SetYPbPrMode525i)
            TimingPoint = pVBInfo->YPbPr525iTiming;

        if (pVBInfo->TVInfo & SetYPbPrMode525p)
            TimingPoint = pVBInfo->YPbPr525pTiming;

        if (pVBInfo->TVInfo & SetYPbPrMode750p)
            TimingPoint = pVBInfo->YPbPr750pTiming;
    }

    for (i = 0x01, j = 0; i <= 0x2D; i++, j++) {
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, i, TimingPoint[j]);
    }

    for (i = 0x39; i <= 0x45; i++, j++) {
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, i, TimingPoint[j]);       /* di->temp2[j] */
    }

    if (pVBInfo->VBInfo & SetCRT2ToTV) {
        XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part2Port, 0x3A, 0x1F, 0x00);
    }

    temp = pVBInfo->NewFlickerMode;
    temp &= 0x80;
    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part2Port, 0x0A, 0xFF, temp);

    if (pVBInfo->VBInfo & SetCRT2ToHiVisionTV)
        tempax = 950;

    if (pVBInfo->TVInfo & SetPALTV)
        tempax = 520;
    else
        tempax = 440;

    if (pVBInfo->VDE <= tempax) {
        tempax -= pVBInfo->VDE;
        tempax = tempax >> 2;
        tempax = (tempax & 0x00FF) | ((tempax & 0x00FF) << 8);
        push1 = tempax;
        temp = (tempax & 0xFF00) >> 8;
        temp += (USHORT) TimingPoint[0];

        if (pVBInfo->
            VBType & (VB_XGI301B | VB_XGI302B | VB_XGI301LV | VB_XGI302LV |
                      VB_XGI301C)) {
            if (pVBInfo->
                VBInfo & (SetCRT2ToAVIDEO | SetCRT2ToSVIDEO | SetCRT2ToSCART |
                          SetCRT2ToYPbPr)) {
                tempcx = pVBInfo->VGAHDE;
                if (tempcx >= 1024) {
                    temp = 0x17;        /* NTSC */
                    if (pVBInfo->TVInfo & SetPALTV)
                        temp = 0x19;    /* PAL */
                }
            }
        }

        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x01, temp);
        tempax = push1;
        temp = (tempax & 0xFF00) >> 8;
        temp += TimingPoint[1];

        if (pVBInfo->
            VBType & (VB_XGI301B | VB_XGI302B | VB_XGI301LV | VB_XGI302LV |
                      VB_XGI301C)) {
            if ((pVBInfo->
                 VBInfo & (SetCRT2ToAVIDEO | SetCRT2ToSVIDEO | SetCRT2ToSCART
                           | SetCRT2ToYPbPr))) {
                tempcx = pVBInfo->VGAHDE;
                if (tempcx >= 1024) {
                    temp = 0x1D;        /* NTSC */
                    if (pVBInfo->TVInfo & SetPALTV)
                        temp = 0x52;    /* PAL */
                }
            }
        }
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x02, temp);
    }

    /* 301b */
    tempcx = pVBInfo->HT;

    if (XGI_IsLCDDualLink(pVBInfo))
        tempcx = tempcx >> 1;

    tempcx -= 2;
    temp = tempcx & 0x00FF;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x1B, temp);

    temp = (tempcx & 0xFF00) >> 8;
    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part2Port, 0x1D, ~0x0F, temp);

    tempcx = pVBInfo->HT >> 1;
    push1 = tempcx;             /* push cx */
    tempcx += 7;

    if (pVBInfo->VBInfo & SetCRT2ToHiVisionTV) {
        tempcx -= 4;
    }

    temp = tempcx & 0x00FF;
    temp = temp << 4;
    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part2Port, 0x22, 0x0F, temp);

    tempbx = TimingPoint[j] | ((TimingPoint[j + 1]) << 8);
    tempbx += tempcx;
    push2 = tempbx;
    temp = tempbx & 0x00FF;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x24, temp);
    temp = (tempbx & 0xFF00) >> 8;
    temp = temp << 4;
    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part2Port, 0x25, 0x0F, temp);

    tempbx = push2;
    tempbx = tempbx + 8;
    if (pVBInfo->VBInfo & SetCRT2ToHiVisionTV) {
        tempbx = tempbx - 4;
        tempcx = tempbx;
    }

    temp = (tempbx & 0x00FF) << 4;
    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part2Port, 0x29, 0x0F, temp);

    j += 2;
    tempcx += (TimingPoint[j] | ((TimingPoint[j + 1]) << 8));
    temp = tempcx & 0x00FF;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x27, temp);
    temp = ((tempcx & 0xFF00) >> 8) << 4;
    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part2Port, 0x28, 0x0F, temp);

    tempcx += 8;
    if (pVBInfo->VBInfo & SetCRT2ToHiVisionTV) {
        tempcx -= 4;
    }

    temp = tempcx & 0xFF;
    temp = temp << 4;
    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part2Port, 0x2A, 0x0F, temp);

    tempcx = push1;             /* pop cx */
    j += 2;
    temp = TimingPoint[j] | ((TimingPoint[j + 1]) << 8);
    tempcx -= temp;
    temp = tempcx & 0x00FF;
    temp = temp << 4;
    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part2Port, 0x2D, 0x0F, temp);

    tempcx -= 11;

    if (!(pVBInfo->VBInfo & SetCRT2ToTV)) {
        tempax = XGI_GetVGAHT2(pVBInfo);
        tempcx = tempax - 1;
    }
    temp = tempcx & 0x00FF;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x2E, temp);

    tempbx = pVBInfo->VDE;

    if (pVBInfo->VGAVDE == 360)
        tempbx = 746;
    if (pVBInfo->VGAVDE == 375)
        tempbx = 746;
    if (pVBInfo->VGAVDE == 405)
        tempbx = 853;

    if (pVBInfo->VBInfo & SetCRT2ToTV) {
        if (pVBInfo->VBType & (VB_XGI301LV | VB_XGI302LV | VB_XGI301C)) {
            if (!(pVBInfo->TVInfo & (SetYPbPrMode525p | SetYPbPrMode750p)))
                tempbx = tempbx >> 1;
        }
        else
            tempbx = tempbx >> 1;
    }

    tempbx -= 2;
    temp = tempbx & 0x00FF;

    if (pVBInfo->VBInfo & SetCRT2ToHiVisionTV) {
        if (pVBInfo->VBType & VB_XGI301LV) {
            if (pVBInfo->TVInfo & SetYPbPrMode1080i) {
                if (pVBInfo->VBInfo & SetInSlaveMode) {
                    if (ModeNo == 0x2f)
                        temp += 1;
                }
            }
        }
        else {
            if (pVBInfo->VBInfo & SetInSlaveMode) {
                if (ModeNo == 0x2f)
                    temp += 1;
            }
        }
    }

    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x2F, temp);

    temp = (tempcx & 0xFF00) >> 8;
    temp |= ((tempbx & 0xFF00) >> 8) << 6;

    if (!(pVBInfo->VBInfo & SetCRT2ToHiVisionTV)) {
        if (pVBInfo->VBType & VB_XGI301LV) {
            if (pVBInfo->TVInfo & SetYPbPrMode1080i) {
                temp |= 0x10;

                if (!(pVBInfo->VBInfo & SetCRT2ToSVIDEO))
                    temp |= 0x20;
            }
        }
        else {
            temp |= 0x10;
            if (!(pVBInfo->VBInfo & SetCRT2ToSVIDEO))
                temp |= 0x20;
        }
    }

    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x30, temp);

    if (pVBInfo->VBType & (VB_XGI301B | VB_XGI302B | VB_XGI301LV | VB_XGI302LV | VB_XGI301C)) { /* TV gatingno */
        tempbx = pVBInfo->VDE;
        tempcx = tempbx - 2;

        if (pVBInfo->VBInfo & SetCRT2ToTV) {
            if (!(pVBInfo->TVInfo & (SetYPbPrMode525p | SetYPbPrMode750p)))
                tempbx = tempbx >> 1;
        }

        if (pVBInfo->VBType & (VB_XGI302LV | VB_XGI301C)) {
            temp = 0;
            if (tempcx & 0x0400)
                temp |= 0x20;

            if (tempbx & 0x0400)
                temp |= 0x40;

            XGI_SetReg((XGIIOADDRESS) pVBInfo->Part4Port, 0x10, temp);
        }

        temp = (((tempbx - 3) & 0x0300) >> 8) << 5;
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x46, temp);
        temp = (tempbx - 3) & 0x00FF;
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x47, temp);
    }

    tempbx = tempbx & 0x00FF;

    if (!(modeflag & HalfDCLK)) {
        tempcx = pVBInfo->VGAHDE;
        if (tempcx >= pVBInfo->HDE) {
            tempbx |= 0x2000;
            tempax &= 0x00FF;
        }
    }

    tempcx = 0x0101;

    if (pVBInfo->VBInfo & SetCRT2ToTV) {        /*301b */
        if (pVBInfo->VGAHDE >= 1024) {
            tempcx = 0x1920;
            if (pVBInfo->VGAHDE >= 1280) {
                tempcx = 0x1420;
                tempbx = tempbx & 0xDFFF;
            }
        }
    }

    if (!(tempbx & 0x2000)) {
        if (modeflag & HalfDCLK) {
            tempcx = (tempcx & 0xFF00) | ((tempcx & 0x00FF) << 1);
        }

        push1 = tempbx;
        tempeax = pVBInfo->VGAHDE;
        tempebx = (tempcx & 0xFF00) >> 8;
        longtemp = tempeax * tempebx;
        tempecx = tempcx & 0x00FF;
        longtemp = longtemp / tempecx;

        /* 301b */
        tempecx = 8 * 1024;

        if (pVBInfo->
            VBType & (VB_XGI301B | VB_XGI302B | VB_XGI301LV | VB_XGI302LV |
                      VB_XGI301C)) {
            tempecx = tempecx * 8;
        }

        longtemp = longtemp * tempecx;
        tempecx = pVBInfo->HDE;
        temp2 = longtemp % tempecx;
        tempeax = longtemp / tempecx;
        if (temp2 != 0) {
            tempeax += 1;
        }

        tempax = (USHORT) tempeax;

        /* 301b */
        if (pVBInfo->
            VBType & (VB_XGI301B | VB_XGI302B | VB_XGI301LV | VB_XGI302LV |
                      VB_XGI301C)) {
            tempcx = ((tempax & 0xFF00) >> 5) >> 8;
        }
        /* end 301b */

        tempbx = push1;
        tempbx =
            (USHORT) (((tempeax & 0x0000FF00) & 0x1F00) | (tempbx & 0x00FF));
        tempax = (USHORT) (((tempeax & 0x000000FF) << 8) | (tempax & 0x00FF));
        temp = (tempax & 0xFF00) >> 8;
    }
    else {
        temp = (tempax & 0x00FF) >> 8;
    }

    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x44, temp);
    temp = (tempbx & 0xFF00) >> 8;
    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part2Port, 0x45, ~0x03F, temp);
    temp = tempcx & 0x00FF;

    if (tempbx & 0x2000)
        temp = 0;

    if (!(pVBInfo->VBInfo & SetCRT2ToLCD))
        temp |= 0x18;

    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part2Port, 0x46, ~0x1F, temp);
    if (pVBInfo->TVInfo & SetPALTV) {
        tempbx = 0x0382;
        tempcx = 0x007e;
    }
    else {
        tempbx = 0x0369;
        tempcx = 0x0061;
    }

    temp = tempbx & 0x00FF;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x4b, temp);
    temp = tempcx & 0x00FF;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x4c, temp);

    temp = ((tempcx & 0xFF00) >> 8) & 0x03;
    temp = temp << 2;
    temp |= ((tempbx & 0xFF00) >> 8) & 0x03;

    if (pVBInfo->VBInfo & SetCRT2ToYPbPr) {
        temp |= 0x10;

        if (pVBInfo->TVInfo & SetYPbPrMode525p)
            temp |= 0x20;

        if (pVBInfo->TVInfo & SetYPbPrMode750p)
            temp |= 0x60;
    }

    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x4d, temp);
    temp = XGI_GetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x43); /* 301b change */
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x43, (USHORT) (temp - 3));

    if (!(pVBInfo->TVInfo & (SetYPbPrMode525p | SetYPbPrMode750p))) {
        if (pVBInfo->TVInfo & NTSC1024x768) {
            TimingPoint = XGI_NTSC1024AdjTime;
            for (i = 0x1c, j = 0; i <= 0x30; i++, j++) {
                XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, i,
                           TimingPoint[j]);
            }
            XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x43, 0x72);
        }
    }

    /* [ycchen] 01/14/03 Modify for 301C PALM Support */
    if (pVBInfo->VBType & VB_XGI301C) {
        if (pVBInfo->TVInfo & SetPALMTV)
            XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part2Port, 0x4E, ~0x08, 0x08);      /* PALM Mode */
    }

    if (pVBInfo->TVInfo & SetPALMTV) {
        tempax = (UCHAR) XGI_GetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x01);
        tempax--;
        XGI_SetRegAND((XGIIOADDRESS) pVBInfo->Part2Port, 0x01, tempax);

        /* if ( !( pVBInfo->VBType & VB_XGI301C ) ) */
        XGI_SetRegAND((XGIIOADDRESS) pVBInfo->Part2Port, 0x00, 0xEF);
    }

    if (pVBInfo->VBInfo & SetCRT2ToHiVisionTV) {
        if (!(pVBInfo->VBInfo & SetInSlaveMode)) {
            XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x0B, 0x00);
        }
    }

    if (pVBInfo->VBInfo & SetCRT2ToTV) {
        return;
    }
    ErrorF("5935 Part2 0 = %x ",
           XGI_GetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x0));
}


/* --------------------------------------------------------------------- */
/* Function : XGI_SetLCDRegs */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_SetLCDRegs(USHORT ModeNo, USHORT ModeIdIndex,
               PXGI_HW_DEVICE_INFO HwDeviceExtension,
               USHORT RefreshRateTableIndex, PVB_DEVICE_INFO pVBInfo)
{
    USHORT push1,
        push2,
        pushbx,
        tempax,
        tempbx,
        tempcx, temp, tempah, tempbh, tempch, resinfo, modeflag, CRT1Index;

    XGI_LCDDesStruct *LCDBDesPtr = NULL;
    XGI330_LCDDataDesStruct2 *LCDPtr1 = NULL;


    if (ModeNo <= 0x13) {
        modeflag = pVBInfo->SModeIDTable[ModeIdIndex].St_ModeFlag;      /* si+St_ResInfo */
        resinfo = pVBInfo->SModeIDTable[ModeIdIndex].St_ResInfo;
    }
    else {
        modeflag = pVBInfo->EModeIDTable[ModeIdIndex].Ext_ModeFlag;     /* si+Ext_ResInfo */
        resinfo = pVBInfo->EModeIDTable[ModeIdIndex].Ext_RESINFO;
        CRT1Index = pVBInfo->RefIndex[RefreshRateTableIndex].Ext_CRT1CRTC;
        CRT1Index &= IndexMask;
    }

    if (!(pVBInfo->VBInfo & SetCRT2ToLCD)) {
        return;
    }

    tempbx = pVBInfo->HDE;      /* RHACTE=HDE-1 */

    if (XGI_IsLCDDualLink(pVBInfo))
        tempbx = tempbx >> 1;

    tempbx -= 1;
    temp = tempbx & 0x00FF;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x2C, temp);
    temp = (tempbx & 0xFF00) >> 8;
    temp = temp << 4;
    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part2Port, 0x2B, 0x0F, temp);
    temp = 0x01;

    if (pVBInfo->LCDResInfo == Panel1280x1024) {
        if (pVBInfo->ModeType == ModeEGA) {
            if (pVBInfo->VGAHDE >= 1024) {
                temp = 0x02;
                if (pVBInfo->LCDInfo & LCDVESATiming)
                    temp = 0x01;
            }
        }
    }

    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x0B, temp);
    tempbx = pVBInfo->VDE;      /* RTVACTEO=(VDE-1)&0xFF */
    push1 = tempbx;
    tempbx--;
    temp = tempbx & 0x00FF;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x03, temp);
    temp = ((tempbx & 0xFF00) >> 8) & 0x07;
    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part2Port, 0x0C, ~0x07, temp);

    tempcx = pVBInfo->VT - 1;
    push2 = tempcx + 1;
    temp = tempcx & 0x00FF;     /* RVTVT=VT-1 */
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x19, temp);
    temp = (tempcx & 0xFF00) >> 8;
    temp = temp << 5;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x1A, temp);
    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part2Port, 0x09, 0xF0, 0x00);
    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part2Port, 0x0A, 0xF0, 0x00);
    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part2Port, 0x17, 0xFB, 0x00);
    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part2Port, 0x18, 0xDF, 0x00);

    /* Customized LCDB Des no add */
    LCDBDesPtr = (XGI_LCDDesStruct *) XGI_GetLcdPtr(5, ModeNo, ModeIdIndex,
						    RefreshRateTableIndex, 
						    pVBInfo);

    if (pVBInfo->LCDInfo & EnableScalingLCD) {
        tempbx = pVBInfo->HDE;
        tempcx = pVBInfo->VDE;
    }
    else {
	get_HDE_VDE(pVBInfo, & tempbx, & tempcx);
    }

    pushbx = tempbx;
    tempax = pVBInfo->VT;
    pVBInfo->LCDHDES = LCDBDesPtr->LCDHDES;
    pVBInfo->LCDHRS = LCDBDesPtr->LCDHRS;
    pVBInfo->LCDVDES = LCDBDesPtr->LCDVDES;
    pVBInfo->LCDVRS = LCDBDesPtr->LCDVRS;
    tempbx = pVBInfo->LCDVDES;
    tempcx += tempbx;

    if (tempcx >= tempax)
        tempcx -= tempax;       /* lcdvdes */

    temp = tempbx & 0x00FF;     /* RVEQ1EQ=lcdvdes */
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x05, temp);
    temp = tempcx & 0x00FF;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x06, temp);
    tempch = ((tempcx & 0xFF00) >> 8) & 0x07;
    tempbh = ((tempbx & 0xFF00) >> 8) & 0x07;
    tempah = tempch;
    tempah = tempah << 3;
    tempah |= tempbh;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x02, tempah);

    /* getlcdsync() */
    XGI_GetLCDSync(&tempax, &tempbx, pVBInfo);
    if (pVBInfo->LCDInfo & EnableScalingLCD) {
        LCDPtr1 =
            (XGI330_LCDDataDesStruct2 *) XGI_GetLcdPtr(4, ModeNo, ModeIdIndex,
                                                       RefreshRateTableIndex,
                                                       pVBInfo);
        tempbx = LCDPtr1->LCDVSync;
    }
    tempcx = tempbx;
    tempax = pVBInfo->VT;
    tempbx = pVBInfo->LCDVRS;

    /* if ( SetLCD_Info & EnableScalingLCD ) */
    tempcx += tempbx;
    if (tempcx >= tempax)
        tempcx -= tempax;

    temp = tempbx & 0x00FF;     /* RTVACTEE=lcdvrs */
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x04, temp);
    temp = (tempbx & 0xFF00) >> 8;
    temp = temp << 4;
    temp |= (tempcx & 0x000F);
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x01, temp);
    tempcx = pushbx;
    tempax = pVBInfo->HT;
    tempbx = pVBInfo->LCDHDES;
    tempbx &= 0x0FFF;

    if (XGI_IsLCDDualLink(pVBInfo)) {
        tempax = tempax >> 1;
        tempbx = tempbx >> 1;
        tempcx = tempcx >> 1;
    }

    if (pVBInfo->VBType & VB_XGI302LV)
        tempbx += 1;

    if (pVBInfo->VBType & VB_XGI301C)   /* tap4 */
        tempbx += 1;

    tempcx += tempbx;

    if (tempcx >= tempax)
        tempcx -= tempax;

    temp = tempbx & 0x00FF;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x1F, temp);  /* RHBLKE=lcdhdes */
    temp = ((tempbx & 0xFF00) >> 8) << 4;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x20, temp);
    temp = tempcx & 0x00FF;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x23, temp);  /* RHEQPLE=lcdhdee */
    temp = (tempcx & 0xFF00) >> 8;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x25, temp);

    /* getlcdsync() */
    XGI_GetLCDSync(&tempax, &tempbx, pVBInfo);
    if (pVBInfo->LCDInfo & EnableScalingLCD) {
        LCDPtr1 =
            (XGI330_LCDDataDesStruct2 *) XGI_GetLcdPtr(4, ModeNo, ModeIdIndex,
                                                       RefreshRateTableIndex,
                                                       pVBInfo);
        tempax = LCDPtr1->LCDHSync;
    }
    tempcx = tempax;
    tempax = pVBInfo->HT;
    tempbx = pVBInfo->LCDHRS;
    /* if ( SetLCD_Info & EnableScalingLCD) */
    if (XGI_IsLCDDualLink(pVBInfo)) {
        tempax = tempax >> 1;
        tempbx = tempbx >> 1;
        tempcx = tempcx >> 1;
    }

    if (pVBInfo->VBType & VB_XGI302LV)
        tempbx += 1;

    tempcx += tempbx;

    if (tempcx >= tempax)
        tempcx -= tempax;

    temp = tempbx & 0x00FF;     /* RHBURSTS=lcdhrs */
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x1C, temp);

    temp = (tempbx & 0xFF00) >> 8;
    temp = temp << 4;
    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part2Port, 0x1D, ~0x0F0, temp);
    temp = tempcx & 0x00FF;     /* RHSYEXP2S=lcdhre */
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x21, temp);

    if (!(pVBInfo->LCDInfo & LCDVESATiming)) {
        if (pVBInfo->VGAVDE == 525) {
            if (pVBInfo->
                VBType & (VB_XGI301B | VB_XGI302B | VB_XGI301LV | VB_XGI302LV
                          | VB_XGI301C)) {
                temp = 0xC6;
            }
            else
                temp = 0xC4;

            XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x2f, temp);
            XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x30, 0xB3);
        }

        if (pVBInfo->VGAVDE == 420) {
            if (pVBInfo->
                VBType & (VB_XGI301B | VB_XGI302B | VB_XGI301LV | VB_XGI302LV
                          | VB_XGI301C)) {
                temp = 0x4F;
            }
            else
                temp = 0x4E;
            XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x2f, temp);
        }
    }
}


/* --------------------------------------------------------------------- */
/* Function : XGI_GetTap4Ptr */
/* Input : */
/* Output : di -> Tap4 Reg. Setting Pointer */
/* Description : */
/* --------------------------------------------------------------------- */
const XGI301C_Tap4TimingStruct *
XGI_GetTap4Ptr(USHORT tempcx, PVB_DEVICE_INFO pVBInfo)
{
    USHORT tempax, tempbx, i;

    const XGI301C_Tap4TimingStruct *Tap4TimingPtr;

    if (tempcx == 0) {
        tempax = pVBInfo->VGAHDE;
        tempbx = pVBInfo->HDE;
    }
    else {
        tempax = pVBInfo->VGAVDE;
        tempbx = pVBInfo->VDE;
    }

    if (tempax < tempbx)
        return &EnlargeTap4Timing[0];
    else if (tempax == tempbx)
        return &NoScaleTap4Timing[0];   /* 1:1 */
    else
        Tap4TimingPtr = NTSCTap4Timing; /* NTSC */

    if (pVBInfo->TVInfo & SetPALTV)
        Tap4TimingPtr = PALTap4Timing;


    if (pVBInfo->VBInfo & SetCRT2ToYPbPr) {
        if (pVBInfo->TVInfo & SetYPbPrMode525i)
            Tap4TimingPtr = YPbPr525iTap4Timing;
        if (pVBInfo->TVInfo & SetYPbPrMode525p)
            Tap4TimingPtr = YPbPr525pTap4Timing;
        if (pVBInfo->TVInfo & SetYPbPrMode750p)
            Tap4TimingPtr = YPbPr750pTap4Timing;
    }

    if (pVBInfo->VBInfo & SetCRT2ToHiVisionTV)
        Tap4TimingPtr = HiTVTap4Timing;

    i = 0;
    while (Tap4TimingPtr[i].DE != 0xFFFF) {
        if (Tap4TimingPtr[i].DE == tempax)
            break;
        i++;
    }
    return &Tap4TimingPtr[i];
}


/* --------------------------------------------------------------------- */
/* Function : XGI_SetTap4Regs */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_SetTap4Regs(PVB_DEVICE_INFO pVBInfo)
{

#ifndef LINUX_XF86
    USHORT tempcx;
#endif
    USHORT i, j;

    const XGI301C_Tap4TimingStruct *Tap4TimingPtr;

    if (!(pVBInfo->VBType & VB_XGI301C))
        return;

#ifndef Tap4
    XGI_SetRegAND((XGIIOADDRESS) pVBInfo->Part2Port, 0x4E, 0xEB);       /* Disable Tap4 */
#else /* Tap4 Setting */

    Tap4TimingPtr = XGI_GetTap4Ptr(0, pVBInfo); /* Set Horizontal Scaling */
    for (i = 0x80, j = 0; i <= 0xBF; i++, j++)
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, i,
                   Tap4TimingPtr->Reg[j]);

    if ((pVBInfo->VBInfo & SetCRT2ToTV)
        && (!(pVBInfo->VBInfo & SetCRT2ToHiVisionTV))) {
        Tap4TimingPtr = XGI_GetTap4Ptr(1, pVBInfo);     /* Set Vertical Scaling */
        for (i = 0xC0, j = 0; i < 0xFF; i++, j++)
            XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, i,
                       Tap4TimingPtr->Reg[j]);
    }

    if ((pVBInfo->VBInfo & SetCRT2ToTV)
        && (!(pVBInfo->VBInfo & SetCRT2ToHiVisionTV)))
        XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part2Port, 0x4E, ~0x14, 0x04);  /* Enable V.Scaling */
    else
        XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part2Port, 0x4E, ~0x14, 0x10);  /* Enable H.Scaling */
#endif
}

/* --------------------------------------------------------------------- */
/* Function : XGI_SetGroup3 */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_SetGroup3(USHORT ModeNo, USHORT ModeIdIndex, PVB_DEVICE_INFO pVBInfo)
{
    USHORT i;
    const UCHAR *tempdi;
    USHORT modeflag;

    if (ModeNo <= 0x13) {
        modeflag = pVBInfo->SModeIDTable[ModeIdIndex].St_ModeFlag;      /* si+St_ResInfo */
    }
    else {
        modeflag = pVBInfo->EModeIDTable[ModeIdIndex].Ext_ModeFlag;     /* si+Ext_ResInfo */
    }


    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part3Port, 0x00, 0x00);
    if (pVBInfo->TVInfo & SetPALTV) {
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part3Port, 0x13, 0xFA);
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part3Port, 0x14, 0xC8);
    }
    else {
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part3Port, 0x13, 0xF5);
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part3Port, 0x14, 0xB7);
    }

    if (!(pVBInfo->VBInfo & SetCRT2ToTV)) {
        return;
    }

    if (pVBInfo->TVInfo & SetPALMTV) {
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part3Port, 0x13, 0xFA);
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part3Port, 0x14, 0xC8);
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part3Port, 0x3D, 0xA8);
    }

    if ((pVBInfo->VBInfo & SetCRT2ToHiVisionTV)
        || (pVBInfo->VBInfo & SetCRT2ToYPbPr)) {
        if (pVBInfo->TVInfo & SetYPbPrMode525i) {
            return;
        }
        tempdi = pVBInfo->HiTVGroup3Data;
        if (pVBInfo->SetFlag & TVSimuMode) {
            tempdi = pVBInfo->HiTVGroup3Simu;
            if (!(modeflag & Charx8Dot)) {
                tempdi = pVBInfo->HiTVGroup3Text;
            }
        }

        if (pVBInfo->TVInfo & SetYPbPrMode525p) {
            tempdi = pVBInfo->Ren525pGroup3;
        }
        if (pVBInfo->TVInfo & SetYPbPrMode750p) {
            tempdi = pVBInfo->Ren750pGroup3;
        }

        for (i = 0; i <= 0x3E; i++) {
            XGI_SetReg((XGIIOADDRESS) pVBInfo->Part3Port, i, tempdi[i]);
        }
        if (pVBInfo->VBType & VB_XGI301C) {     /* Marcovision */
            if (pVBInfo->TVInfo & SetYPbPrMode525p) {
                XGI_SetReg((XGIIOADDRESS) pVBInfo->Part3Port, 0x28, 0x3f);
            }
        }
    }
    return;
}                               /* {end of XGI_SetGroup3} */


/* --------------------------------------------------------------------- */
/* Function : XGI_SetGroup4 */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_SetGroup4(USHORT ModeNo, USHORT ModeIdIndex, USHORT RefreshRateTableIndex,
              PXGI_HW_DEVICE_INFO HwDeviceExtension, PVB_DEVICE_INFO pVBInfo)
{
    USHORT tempax, tempcx, tempbx, modeflag, temp, temp2;
#ifndef LINUX_XF86
    USHORT push1;
#endif

    ULONG tempebx, tempeax, templong;


    if (ModeNo <= 0x13) {
        modeflag = pVBInfo->SModeIDTable[ModeIdIndex].St_ModeFlag;      /* si+St_ResInfo */
    }
    else {
        modeflag = pVBInfo->EModeIDTable[ModeIdIndex].Ext_ModeFlag;     /* si+Ext_ResInfo */
    }

    temp = pVBInfo->RVBHCFACT;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part4Port, 0x13, temp);

    tempbx = pVBInfo->RVBHCMAX;
    temp = tempbx & 0x00FF;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part4Port, 0x14, temp);
    temp2 = ((tempbx & 0xFF00) >> 8) << 7;
    tempcx = pVBInfo->VGAHT - 1;
    temp = tempcx & 0x00FF;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part4Port, 0x16, temp);

    temp = ((tempcx & 0xFF00) >> 8) << 3;
    temp2 |= temp;

    tempcx = pVBInfo->VGAVT - 1;
    if (!(pVBInfo->VBInfo & SetCRT2ToTV)) {
        tempcx -= 5;
    }

    temp = tempcx & 0x00FF;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part4Port, 0x17, temp);
    temp = temp2 | ((tempcx & 0xFF00) >> 8);
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part4Port, 0x15, temp);
    XGI_SetRegOR((XGIIOADDRESS) pVBInfo->Part4Port, 0x0D, 0x08);
    tempcx = pVBInfo->VBInfo;
    tempbx = pVBInfo->VGAHDE;

    if (modeflag & HalfDCLK) {
        tempbx = tempbx >> 1;
    }

    if (XGI_IsLCDDualLink(pVBInfo))
        tempbx = tempbx >> 1;

    if (tempcx & SetCRT2ToHiVisionTV) {
        temp = 0;
        if (tempbx <= 1024)
            temp = 0xA0;
        if (tempbx == 1280)
            temp = 0xC0;
    }
    else if (tempcx & SetCRT2ToTV) {
        temp = 0xA0;
        if (tempbx <= 800)
            temp = 0x80;
    }
    else {
        temp = 0x80;
        if (pVBInfo->VBInfo & SetCRT2ToLCD) {
            temp = 0;
            if (tempbx > 800)
                temp = 0x60;
        }
    }

    if (pVBInfo->TVInfo & (SetYPbPrMode525p | SetYPbPrMode750p)) {
        temp = 0x00;
        if (pVBInfo->VGAHDE == 1280)
            temp = 0x40;
        if (pVBInfo->VGAHDE == 1024)
            temp = 0x20;
    }
    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part4Port, 0x0E, ~0xEF, temp);

    tempebx = pVBInfo->VDE;

    if (tempcx & SetCRT2ToHiVisionTV) {
        if (!(temp & 0xE000))
            tempbx = tempbx >> 1;
    }

    tempcx = pVBInfo->RVBHRS;
    temp = tempcx & 0x00FF;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part4Port, 0x18, temp);

    tempeax = pVBInfo->VGAVDE;
    tempcx |= 0x04000;


    if (tempeax <= tempebx) {
        tempcx = (tempcx & (~0x4000));
        tempeax = pVBInfo->VGAVDE;
    }
    else {
        tempeax -= tempebx;
    }


    templong = (tempeax * 256 * 1024) % tempebx;
    tempeax = (tempeax * 256 * 1024) / tempebx;
    tempebx = tempeax;

    if (templong != 0) {
        tempebx++;
    }


    temp = (USHORT) (tempebx & 0x000000FF);
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part4Port, 0x1B, temp);

    temp = (USHORT) ((tempebx & 0x0000FF00) >> 8);
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part4Port, 0x1A, temp);
    tempbx = (USHORT) (tempebx >> 16);
    temp = tempbx & 0x00FF;
    temp = temp << 4;
    temp |= ((tempcx & 0xFF00) >> 8);
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part4Port, 0x19, temp);

    /* 301b */
    if (pVBInfo->
        VBType & (VB_XGI301B | VB_XGI302B | VB_XGI301LV | VB_XGI302LV |
                  VB_XGI301C)) {
        temp = 0x0028;
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part4Port, 0x1C, temp);
        tempax = pVBInfo->VGAHDE;
        if (modeflag & HalfDCLK) {
            tempax = tempax >> 1;
        }

        if (XGI_IsLCDDualLink(pVBInfo))
            tempax = tempax >> 1;

        /* if((pVBInfo->VBInfo&(SetCRT2ToLCD))||((pVBInfo->TVInfo&SetYPbPrMode525p)||(pVBInfo->TVInfo&SetYPbPrMode750p))) { */
        if (pVBInfo->VBInfo & SetCRT2ToLCD) {
            if (tempax > 800)
                tempax -= 800;
        }
        else {
            if (pVBInfo->VGAHDE > 800) {
                if (pVBInfo->VGAHDE == 1024)
                    tempax = (tempax * 25 / 32) - 1;
                else
                    tempax = (tempax * 20 / 32) - 1;
            }
        }
        tempax -= 1;

/*
        if ( pVBInfo->VBInfo & ( SetCRT2ToTV | SetCRT2ToHiVisionTV ) )
        {
            if ( pVBInfo->VBType & VB_XGI301LV )
            {
                if ( !( pVBInfo->TVInfo & ( SetYPbPrMode525p | SetYPbPrMode750p | SetYPbPrMode1080i ) ) )
                {
                    if ( pVBInfo->VGAHDE > 800 )
                    {
                        if ( pVBInfo->VGAHDE == 1024 )
                            tempax = ( tempax * 25 / 32 ) - 1 ;
                        else
                            tempax = ( tempax * 20 / 32 ) - 1 ;
                    }
                }
            }
            else
            {
                if ( pVBInfo->VGAHDE > 800 )
                {
                    if ( pVBInfo->VGAHDE == 1024 )
                        tempax = ( tempax * 25 / 32 ) - 1 ;
                    else
                        tempax = ( tempax * 20 / 32 ) - 1 ;
                }
            }
        }
*/

        temp = (tempax & 0xFF00) >> 8;
        temp = ((temp & 0x0003) << 4);
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part4Port, 0x1E, temp);
        temp = (tempax & 0x00FF);
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part4Port, 0x1D, temp);

        if (pVBInfo->VBInfo & (SetCRT2ToTV | SetCRT2ToHiVisionTV)) {
            if (pVBInfo->VGAHDE > 800) {
                XGI_SetRegOR((XGIIOADDRESS) pVBInfo->Part4Port, 0x1E, 0x08);
            }
        }
        temp = 0x0036;

        if (pVBInfo->VBInfo & SetCRT2ToTV) {
            if (!
                (pVBInfo->
                 TVInfo & (NTSC1024x768 | SetYPbPrMode525p | SetYPbPrMode750p
                           | SetYPbPrMode1080i))) {
                temp |= 0x0001;
                if ((pVBInfo->VBInfo & SetInSlaveMode)
                    && (!(pVBInfo->TVInfo & TVSimuMode)))
                    temp &= (~0x0001);
            }
        }

        XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part4Port, 0x1F, 0x00C0,
                        temp);
        tempbx = pVBInfo->HT;
        if (XGI_IsLCDDualLink(pVBInfo))
            tempbx = tempbx >> 1;
        tempbx = (tempbx >> 1) - 2;
        temp = ((tempbx & 0x0700) >> 8) << 3;
        XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part4Port, 0x21, 0x00C0,
                        temp);
        temp = tempbx & 0x00FF;
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part4Port, 0x22, temp);
    }
    /* end 301b */

    if (pVBInfo->ISXPDOS == 0)
        XGI_SetCRT2VCLK(ModeNo, ModeIdIndex, RefreshRateTableIndex, pVBInfo);
}


/* --------------------------------------------------------------------- */
/* Function : XGI_SetGroup5 */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_SetGroup5(USHORT ModeNo, USHORT ModeIdIndex, PVB_DEVICE_INFO pVBInfo)
{
    USHORT Pindex, Pdata;

    Pindex = pVBInfo->Part5Port;
    Pdata = pVBInfo->Part5Port + 1;
    if (pVBInfo->ModeType == ModeVGA) {
        if (!
            (pVBInfo->
             VBInfo & (SetInSlaveMode | LoadDACFlag | CRT2DisplayFlag))) {
            XGINew_EnableCRT2(pVBInfo);
            /* LoadDAC2(pVBInfo->Part5Port,ModeNo,ModeIdIndex); */
        }
    }
    return;
}


/* --------------------------------------------------------------------- */
/* Function : XGI_GetLcdPtr */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
const void *
XGI_GetLcdPtr(USHORT BX, USHORT ModeNo, USHORT ModeIdIndex,
              USHORT RefreshRateTableIndex, PVB_DEVICE_INFO pVBInfo)
{
    USHORT i, tempdx, tempcx, tempbx, tempal, modeflag, table;

    const XGI330_LCDDataTablStruct *tempdi = 0;


    tempbx = BX;

    if (ModeNo <= 0x13) {
        modeflag = pVBInfo->SModeIDTable[ModeIdIndex].St_ModeFlag;
        tempal = pVBInfo->SModeIDTable[ModeIdIndex].St_CRT2CRTC;
    }
    else {
        modeflag = pVBInfo->EModeIDTable[ModeIdIndex].Ext_ModeFlag;
        tempal = pVBInfo->RefIndex[RefreshRateTableIndex].Ext_CRT2CRTC;
    }

    if (pVBInfo->LCDInfo & EnableScalingLCD) {  /* ScaleLCD */
        if (pVBInfo->EModeIDTable[ModeIdIndex].Ext_RESINFO == 0x14) {
            tempal = 0x0A;
        }
        else if (pVBInfo->EModeIDTable[ModeIdIndex].Ext_RESINFO == 0x0F) {
            tempal = 0x0B;
        }
    }

    tempal = tempal & 0x0f;

    if (tempbx <= 1) {          /* ExpLink */
        if (ModeNo <= 0x13) {
            tempal = pVBInfo->SModeIDTable[ModeIdIndex].St_CRT2CRTC;    /* find no Ext_CRT2CRTC2 */
        }
        else {
            tempal = pVBInfo->RefIndex[RefreshRateTableIndex].Ext_CRT2CRTC;
        }

        if (pVBInfo->VBInfo & SetCRT2ToLCDA) {
            if (ModeNo <= 0x13)
                tempal = pVBInfo->SModeIDTable[ModeIdIndex].St_CRT2CRTC2;
            else
                tempal =
                    pVBInfo->RefIndex[RefreshRateTableIndex].Ext_CRT2CRTC2;
        }

        if (tempbx & 0x01)
            tempal = (tempal >> 4);

        tempal = (tempal & 0x0f);
    }

    tempcx = LCDLenList[tempbx];        /* mov cl,byte ptr cs:LCDLenList[bx] */

    if (pVBInfo->LCDInfo & EnableScalingLCD) {  /* ScaleLCD */
        if ((tempbx == 5) || (tempbx) == 7)
            tempcx = LCDDesDataLen2;
        else if ((tempbx == 3) || (tempbx == 8))
            tempcx = LVDSDesDataLen2;
    }
    /* mov di, word ptr cs:LCDDataList[bx] */
    /* tempdi=pVideoMemory[LCDDataList+tempbx*2]|(pVideoMemory[LCDDataList+tempbx*2+1]<<8); */

    switch (tempbx) {
    case 0:
        tempdi = XGI_EPLLCDCRT1Ptr_H;
        break;
    case 1:
        tempdi = XGI_EPLLCDCRT1Ptr_V;
        break;
    case 2:
        tempdi = XGI_EPLLCDDataPtr;
        break;
    case 3:
        tempdi = XGI_EPLLCDDesDataPtr;
        break;
    case 4:
        tempdi = XGI_LCDDataTable;
        break;
    case 5:
        tempdi = XGI_LCDDesDataTable;
        break;
    case 6:
        tempdi = XGI_EPLCHLCDRegPtr;
        break;
    case 7:
    case 8:
    case 9:
        tempdi = 0;
        break;
    default:
        break;
    }

    if (tempdi == 0x00)         /* OEMUtil */
        return 0;

    table = tempbx;
    i = 0;

    while (tempdi[i].PANELID != 0xff) {
        tempdx = pVBInfo->LCDResInfo;
        if (tempbx & 0x0080) {  /* OEMUtil */
            tempbx &= (~0x0080);
            tempdx = pVBInfo->LCDTypeInfo;
        }

        if (pVBInfo->LCDInfo & EnableScalingLCD) {
            if ((pVBInfo->LCDInfo & EnableReduceTiming)
                && (pVBInfo->LCDResInfo == Panel1600x1200)) {
                tempdx = Panel1600x1200_1;
            }
            else {
                tempdx &= (~PanelResInfo);
            }
        }
        if (tempdi[i].PANELID == tempdx) {
            tempbx = tempdi[i].MASK;
            tempdx = pVBInfo->LCDInfo;

            if (ModeNo <= 0x13) /* alan 09/10/2003 */
                tempdx |= SetLCDStdMode;

            if (modeflag & HalfDCLK)
                tempdx |= SetLCDLowResolution;

            tempbx &= tempdx;
            if (tempbx == tempdi[i].CAP)
                break;
        }
        i++;
    }

    if (table == 0) {
        switch (tempdi[i].DATAPTR) {
        case 0:
            return &XGI_LVDSCRT11024x768_1_H[tempal];
            break;
        case 1:
            return &XGI_LVDSCRT11024x768_2_H[tempal];
            break;
        case 2:
            return &XGI_LVDSCRT11280x1024_1_H[tempal];
            break;
        case 3:
            return &XGI_LVDSCRT11280x1024_2_H[tempal];
            break;
        case 4:
            return &XGI_LVDSCRT11400x1050_1_H[tempal];
            break;
        case 5:
            return &XGI_LVDSCRT11400x1050_2_H[tempal];
            break;
        case 6:
            return &XGI_LVDSCRT11600x1200_1_H[tempal];
            break;
        case 7:
            return &XGI_LVDSCRT11024x768_1_Hx75[tempal];
            break;
        case 8:
            return &XGI_LVDSCRT11024x768_2_Hx75[tempal];
            break;
        case 9:
            return &XGI_LVDSCRT11280x1024_1_Hx75[tempal];
            break;
        case 10:
            return &XGI_LVDSCRT11280x1024_2_Hx75[tempal];
            break;
        default:
            break;
        }
    }
    else if (table == 1) {
        switch (tempdi[i].DATAPTR) {
        case 0:
            return &XGI_LVDSCRT11024x768_1_V[tempal];
            break;
        case 1:
            return &XGI_LVDSCRT11024x768_2_V[tempal];
            break;
        case 2:
            return &XGI_LVDSCRT11280x1024_1_V[tempal];
            break;
        case 3:
            return &XGI_LVDSCRT11280x1024_2_V[tempal];
            break;
        case 4:
            return &XGI_LVDSCRT11400x1050_1_V[tempal];
            break;
        case 5:
            return &XGI_LVDSCRT11400x1050_2_V[tempal];
            break;
        case 6:
            return &XGI_LVDSCRT11600x1200_1_V[tempal];
            break;
        case 7:
            return &XGI_LVDSCRT11024x768_1_Vx75[tempal];
            break;
        case 8:
            return &XGI_LVDSCRT11024x768_2_Vx75[tempal];
            break;
        case 9:
            return &XGI_LVDSCRT11280x1024_1_Vx75[tempal];
            break;
        case 10:
            return &XGI_LVDSCRT11280x1024_2_Vx75[tempal];
            break;
        default:
            break;
        }
    }
    else if (table == 2) {
        switch (tempdi[i].DATAPTR) {
        case 0:
            return &XGI_LVDS1024x768Data_1[tempal];
            break;
        case 1:
            return &XGI_LVDS1024x768Data_2[tempal];
            break;
        case 2:
            return &XGI_LVDS1280x1024Data_1[tempal];
            break;
        case 3:
            return &XGI_LVDS1280x1024Data_2[tempal];
            break;
        case 4:
            return &XGI_LVDS1400x1050Data_1[tempal];
            break;
        case 5:
            return &XGI_LVDS1400x1050Data_2[tempal];
            break;
        case 6:
            return &XGI_LVDS1600x1200Data_1[tempal];
            break;
        case 7:
            return &XGI_LVDSNoScalingData[tempal];
            break;
        case 8:
            return &XGI_LVDS1024x768Data_1x75[tempal];
            break;
        case 9:
            return &XGI_LVDS1024x768Data_2x75[tempal];
            break;
        case 10:
            return &XGI_LVDS1280x1024Data_1x75[tempal];
            break;
        case 11:
            return &XGI_LVDS1280x1024Data_2x75[tempal];
            break;
        case 12:
            return &XGI_LVDSNoScalingDatax75[tempal];
            break;
        default:
            break;
        }
    }
    else if (table == 3) {
        switch (tempdi[i].DATAPTR) {
        case 0:
            return &XGI_LVDS1024x768Des_1[tempal];
            break;
        case 1:
            return &XGI_LVDS1024x768Des_3[tempal];
            break;
        case 2:
            return &XGI_LVDS1024x768Des_2[tempal];
            break;
        case 3:
            return &XGI_LVDS1280x1024Des_1[tempal];
            break;
        case 4:
            return &XGI_LVDS1280x1024Des_2[tempal];
            break;
        case 5:
            return &XGI_LVDS1400x1050Des_1[tempal];
            break;
        case 6:
            return &XGI_LVDS1400x1050Des_2[tempal];
            break;
        case 7:
            return &XGI_LVDS1600x1200Des_1[tempal];
            break;
        case 8:
            return &XGI_LVDSNoScalingDesData[tempal];
            break;
        case 9:
            return &XGI_LVDS1024x768Des_1x75[tempal];
            break;
        case 10:
            return &XGI_LVDS1024x768Des_3x75[tempal];
            break;
        case 11:
            return &XGI_LVDS1024x768Des_2x75[tempal];
            break;
        case 12:
            return &XGI_LVDS1280x1024Des_1x75[tempal];
            break;
        case 13:
            return &XGI_LVDS1280x1024Des_2x75[tempal];
            break;
        case 14:
            return &XGI_LVDSNoScalingDesDatax75[tempal];
            break;
        default:
            break;
        }
    }
    else if (table == 4) {
        switch (tempdi[i].DATAPTR) {
        case 0:
            return &XGI_ExtLCD1024x768Data[tempal];
            break;
        case 1:
            return &XGI_StLCD1024x768Data[tempal];
            break;
        case 2:
            return &XGI_CetLCD1024x768Data[tempal];
            break;
        case 3:
            return &XGI_ExtLCD1280x1024Data[tempal];
            break;
        case 4:
            return &XGI_StLCD1280x1024Data[tempal];
            break;
        case 5:
            return &XGI_CetLCD1280x1024Data[tempal];
            break;
        case 6:
            return &XGI_ExtLCD1400x1050Data[tempal];
            break;
        case 7:
            return &XGI_StLCD1400x1050Data[tempal];
            break;
        case 8:
            return &XGI_CetLCD1400x1050Data[tempal];
            break;
        case 9:
            return &XGI_ExtLCD1600x1200Data[tempal];
            break;
        case 10:
            return &XGI_StLCD1600x1200Data[tempal];
            break;
        case 11:
            return &XGI_NoScalingData[tempal];
            break;
        case 12:
            return &XGI_ExtLCD1024x768x75Data[tempal];
            break;
        case 13:
            return &XGI_ExtLCD1024x768x75Data[tempal];
            break;
        case 14:
            return &XGI_CetLCD1024x768x75Data[tempal];
            break;
        case 15:
            return &XGI_ExtLCD1280x1024x75Data[tempal];
            break;
        case 16:
            return &XGI_StLCD1280x1024x75Data[tempal];
            break;
        case 17:
            return &XGI_CetLCD1280x1024x75Data[tempal];
            break;
        case 18:
            return &XGI_NoScalingDatax75[tempal];
            break;
        case 19:
            return &XGI_NoScalingData_1[tempal];
            break;
        default:
            break;
        }
    }
    else if (table == 5) {
        switch (tempdi[i].DATAPTR) {
        case 0:
            return &XGI_ExtLCDDes1024x768Data[tempal];
            break;
        case 1:
            return &XGI_StLCDDes1024x768Data[tempal];
            break;
        case 2:
            return &XGI_CetLCDDes1024x768Data[tempal];
            break;
        case 3:
            if ((pVBInfo->VBType & VB_XGI301LV)
                || (pVBInfo->VBType & VB_XGI302LV))
                return &XGI_ExtLCDDLDes1280x1024Data[tempal];
            else
                return &XGI_ExtLCDDes1280x1024Data[tempal];
            break;
        case 4:
            if ((pVBInfo->VBType & VB_XGI301LV)
                || (pVBInfo->VBType & VB_XGI302LV))
                return &XGI_StLCDDLDes1280x1024Data[tempal];
            else
                return &XGI_StLCDDes1280x1024Data[tempal];
            break;
        case 5:
            if ((pVBInfo->VBType & VB_XGI301LV)
                || (pVBInfo->VBType & VB_XGI302LV))
                return &XGI_CetLCDDLDes1280x1024Data[tempal];
            else
                return &XGI_CetLCDDes1280x1024Data[tempal];
            break;
        case 6:
            if ((pVBInfo->VBType & VB_XGI301LV)
                || (pVBInfo->VBType & VB_XGI302LV))
                return &XGI_ExtLCDDLDes1400x1050Data[tempal];
            else
                return &XGI_ExtLCDDes1400x1050Data[tempal];
            break;
        case 7:
            if ((pVBInfo->VBType & VB_XGI301LV)
                || (pVBInfo->VBType & VB_XGI302LV))
                return &XGI_StLCDDLDes1400x1050Data[tempal];
            else
                return &XGI_StLCDDes1400x1050Data[tempal];
            break;
        case 8:
            return &XGI_CetLCDDes1400x1050Data[tempal];
            break;
        case 9:
            return &XGI_CetLCDDes1400x1050Data2[tempal];
            break;
        case 10:
            if ((pVBInfo->VBType & VB_XGI301LV)
                || (pVBInfo->VBType & VB_XGI302LV))
                return &XGI_ExtLCDDLDes1600x1200Data[tempal];
            else
                return &XGI_ExtLCDDes1600x1200Data[tempal];
            break;
        case 11:
            if ((pVBInfo->VBType & VB_XGI301LV)
                || (pVBInfo->VBType & VB_XGI302LV))
                return &XGI_StLCDDLDes1600x1200Data[tempal];
            else
                return &XGI_StLCDDes1600x1200Data[tempal];
            break;
        case 12:
            return &XGI_NoScalingDesData[tempal];
            break;
        case 13:
            return &XGI_ExtLCDDes1024x768x75Data[tempal];
            break;
        case 14:
            return &XGI_StLCDDes1024x768x75Data[tempal];
            break;
        case 15:
            return &XGI_CetLCDDes1024x768x75Data[tempal];
            break;
        case 16:
            if ((pVBInfo->VBType & VB_XGI301LV)
                || (pVBInfo->VBType & VB_XGI302LV))
                return &XGI_ExtLCDDLDes1280x1024x75Data[tempal];
            else
                return &XGI_ExtLCDDes1280x1024x75Data[tempal];
            break;
        case 17:
            if ((pVBInfo->VBType & VB_XGI301LV)
                || (pVBInfo->VBType & VB_XGI302LV))
                return &XGI_StLCDDLDes1280x1024x75Data[tempal];
            else
                return &XGI_StLCDDes1280x1024x75Data[tempal];
            break;
        case 18:
            if ((pVBInfo->VBType & VB_XGI301LV)
                || (pVBInfo->VBType & VB_XGI302LV))
                return &XGI_CetLCDDLDes1280x1024x75Data[tempal];
            else
                return &XGI_CetLCDDes1280x1024x75Data[tempal];
            break;
        case 19:
            return &XGI_NoScalingDesDatax75[tempal];
            break;
        case 20:
            return &XGI_NoScalingDesData_1[tempal];
            break;
        default:
            break;
        }
    }
    else if (table == 6) {
        switch (tempdi[i].DATAPTR) {
        case 0:
            return &XGI_CH7017LV1024x768[tempal];
            break;
        case 1:
            return &XGI_CH7017LV1400x1050[tempal];
            break;
        default:
            break;
        }
    }
    return 0;
}


/* --------------------------------------------------------------------- */
/* Function : XGI_GetTVPtr */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
const void *
XGI_GetTVPtr(USHORT BX, USHORT ModeNo, USHORT ModeIdIndex,
             USHORT RefreshRateTableIndex, PVB_DEVICE_INFO pVBInfo)
{
    USHORT i, tempdx, tempbx, tempal, modeflag, table;
    const XGI330_TVDataTablStruct *tempdi = NULL;

    tempbx = BX;

    if (ModeNo <= 0x13) {
        modeflag = pVBInfo->SModeIDTable[ModeIdIndex].St_ModeFlag;
        tempal = pVBInfo->SModeIDTable[ModeIdIndex].St_CRT2CRTC;
    }
    else {
        modeflag = pVBInfo->EModeIDTable[ModeIdIndex].Ext_ModeFlag;
        tempal = pVBInfo->RefIndex[RefreshRateTableIndex].Ext_CRT2CRTC;
    }

    tempal = tempal & 0x3f;
    table = tempbx;

    switch (tempbx) {
    case 0:
        tempdi = 0;             /*EPLCHTVCRT1Ptr_H; */
        break;
    case 1:
        tempdi = 0;             /*EPLCHTVCRT1Ptr_V; */
        break;
    case 2:
        tempdi = XGI_EPLCHTVDataPtr;
        break;
    case 3:
        tempdi = 0;
        break;
    case 4:
        tempdi = XGI_TVDataTable;
        break;
    case 5:
        tempdi = 0;
        break;
    case 6:
        tempdi = XGI_EPLCHTVRegPtr;
        break;
    default:
        break;
    }

    if (tempdi == 0x00)         /* OEMUtil */
        return (0);

    tempdx = pVBInfo->TVInfo;

    if (pVBInfo->VBInfo & SetInSlaveMode)
        tempdx = tempdx | SetTVLockMode;

    if (modeflag & HalfDCLK)
        tempdx = tempdx | SetTVLowResolution;

    i = 0;

    while (tempdi[i].MASK != 0xffff) {
        if ((tempdx & tempdi[i].MASK) == tempdi[i].CAP)
            break;
        i++;
    }

    if (table == 0x04) {
        switch (tempdi[i].DATAPTR) {
        case 0:
            return &XGI_ExtPALData[tempal];
            break;
        case 1:
            return &XGI_ExtNTSCData[tempal];
            break;
        case 2:
            return &XGI_StPALData[tempal];
            break;
        case 3:
            return &XGI_StNTSCData[tempal];
            break;
        case 4:
            return &XGI_ExtHiTVData[tempal];
            break;
        case 5:
            return &XGI_St2HiTVData[tempal];
            break;
        case 6:
            return &XGI_ExtYPbPr525iData[tempal];
            break;
        case 7:
            return &XGI_ExtYPbPr525pData[tempal];
            break;
        case 8:
            return &XGI_ExtYPbPr750pData[tempal];
            break;
        case 9:
            return &XGI_StYPbPr525iData[tempal];
            break;
        case 10:
            return &XGI_StYPbPr525pData[tempal];
            break;
        case 11:
            return &XGI_StYPbPr750pData[tempal];
            break;
        case 12:               /* avoid system hang */
            return &XGI_ExtNTSCData[tempal];
            break;
        case 13:
            return &XGI_St1HiTVData[tempal];
            break;
        default:
            break;
        }
    }
    else if (table == 0x02) {
        switch (tempdi[i].DATAPTR) {
        case 0:
            return &XGI_CHTVUNTSCData[tempal];
            break;
        case 1:
            return &XGI_CHTVONTSCData[tempal];
            break;
        case 2:
            return &XGI_CHTVUPALData[tempal];
            break;
        case 3:
            return &XGI_CHTVOPALData[tempal];
            break;
        default:
            break;
        }
    }
    else if (table == 0x06) {
        switch (tempdi[i].DATAPTR) {
        case 0:
            return &XGI_CHTVRegUNTSC[tempal];
            break;
        case 1:
            return &XGI_CHTVRegONTSC[tempal];
            break;
        case 2:
            return &XGI_CHTVRegUPAL[tempal];
            break;
        case 3:
            return &XGI_CHTVRegOPAL[tempal];
            break;
        default:
            break;
        }
    }
    return (0);
}


/* --------------------------------------------------------------------- */
/* Function : XGI_BacklightByDrv */
/* Input : */
/* Output : TRUE -> Skip backlight control */
/* Description : */
/* --------------------------------------------------------------------- */
BOOLEAN
XGI_BacklightByDrv(PVB_DEVICE_INFO pVBInfo)
{
    UCHAR tempah;

    tempah = (UCHAR) XGI_GetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x3A);
    if (tempah & BacklightControlBit)
        return TRUE;
    else
        return FALSE;
}


/* --------------------------------------------------------------------- */
/* Function : XGI_FirePWDDisable */
/* Input : */
/* Output : */
/* Description : Turn off VDD & Backlight : Fire disable procedure */
/* --------------------------------------------------------------------- */
/*
void XGI_FirePWDDisable( PVB_DEVICE_INFO pVBInfo )
{
    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part1Port , 0x26 , 0x00 , 0xFC ) ;
}
*/

/* --------------------------------------------------------------------- */
/* Function : XGI_FirePWDEnable */
/* Input : */
/* Output : */
/* Description : Turn on VDD & Backlight : Fire enable procedure */
/* --------------------------------------------------------------------- */
void
XGI_FirePWDEnable(PVB_DEVICE_INFO pVBInfo)
{
    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part1Port, 0x26, 0x03, 0xFC);
}


/* --------------------------------------------------------------------- */
/* Function : XGI_EnableGatingCRT */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_EnableGatingCRT(PXGI_HW_DEVICE_INFO HwDeviceExtension,
                    PVB_DEVICE_INFO pVBInfo)
{
    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->P3d4, 0x63, 0xBF, 0x40);
}


/* --------------------------------------------------------------------- */
/* Function : XGI_DisableGatingCRT */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_DisableGatingCRT(PXGI_HW_DEVICE_INFO HwDeviceExtension,
                     PVB_DEVICE_INFO pVBInfo)
{

    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->P3d4, 0x63, 0xBF, 0x00);
}


/* --------------------------------------------------------------------- */
/* Function : XGI_SetPanelDelay */
/* Input : */
/* Output : */
/* Description : */
/* I/P : bl : 1 ; T1 : the duration between CPL on and signal on */
/* : bl : 2 ; T2 : the duration signal on and Vdd on */
/* : bl : 3 ; T3 : the duration between CPL off and signal off */
/* : bl : 4 ; T4 : the duration signal off and Vdd off */
/* --------------------------------------------------------------------- */
void
XGI_SetPanelDelay(USHORT tempbl, PVB_DEVICE_INFO pVBInfo)
{
    USHORT index;
#ifndef LINUX_XF86
    USHORT temp;
#endif

    index = XGI_GetLCDCapPtr(pVBInfo);

    if (tempbl == 1)
        XGINew_LCD_Wait_Time(pVBInfo->LCDCapList[index].PSC_S1, pVBInfo);

    if (tempbl == 2)
        XGINew_LCD_Wait_Time(pVBInfo->LCDCapList[index].PSC_S2, pVBInfo);

    if (tempbl == 3)
        XGINew_LCD_Wait_Time(pVBInfo->LCDCapList[index].PSC_S3, pVBInfo);

    if (tempbl == 4)
        XGINew_LCD_Wait_Time(pVBInfo->LCDCapList[index].PSC_S4, pVBInfo);
}


/* --------------------------------------------------------------------- */
/* Function : XGI_SetPanelPower */
/* Input : */
/* Output : */
/* Description : */
/* I/O : ah = 0011b = 03h ; Backlight on, Power on */
/* = 0111b = 07h ; Backlight on, Power off */
/* = 1011b = 0Bh ; Backlight off, Power on */
/* = 1111b = 0Fh ; Backlight off, Power off */
/* --------------------------------------------------------------------- */
void
XGI_SetPanelPower(USHORT tempah, USHORT tempbl, PVB_DEVICE_INFO pVBInfo)
{
    if (pVBInfo->VBType & (VB_XGI301LV | VB_XGI302LV | VB_XGI301C))
        XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part4Port, 0x26, tempbl,
                        tempah);
    else
        XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->P3c4, 0x11, tempbl, tempah);
}


/* --------------------------------------------------------------------- */
/* Function : XGI_IsLCDON */
/* Input : */
/* Output : FALSE : Skip PSC Control */
/* TRUE: Disable PSC */
/* Description : */
/* --------------------------------------------------------------------- */
BOOLEAN
XGI_IsLCDON(PVB_DEVICE_INFO pVBInfo)
{
    USHORT tempax;

    tempax = pVBInfo->VBInfo;
    if (tempax & SetCRT2ToDualEdge)
        return FALSE;
    else if (tempax & (DisableCRT2Display | SwitchToCRT2 | SetSimuScanMode))
        return TRUE;

    return FALSE;
}


/* --------------------------------------------------------------------- */
/* Function : XGI_EnablePWD */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_EnablePWD(PVB_DEVICE_INFO pVBInfo)
{
    USHORT index, temp;

    index = XGI_GetLCDCapPtr(pVBInfo);
    temp = pVBInfo->LCDCapList[index].PWD_2B;
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part4Port, 0x2B, temp);
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part4Port, 0x2C,
               pVBInfo->LCDCapList[index].PWD_2C);
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part4Port, 0x2D,
               pVBInfo->LCDCapList[index].PWD_2D);
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part4Port, 0x2E,
               pVBInfo->LCDCapList[index].PWD_2E);
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part4Port, 0x2F,
               pVBInfo->LCDCapList[index].PWD_2F);
    XGI_SetRegOR((XGIIOADDRESS) pVBInfo->Part4Port, 0x27, 0x80);        /* enable PWD */
}


/* --------------------------------------------------------------------- */
/* Function : XGI_DisablePWD */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_DisablePWD(PVB_DEVICE_INFO pVBInfo)
{
    XGI_SetRegAND((XGIIOADDRESS) pVBInfo->Part4Port, 0x27, 0x7F);       /* disable PWD */
}


/* --------------------------------------------------------------------- */
/* Function : XGI_DisableChISLCD */
/* Input : */
/* Output : FALSE -> Not LCD Mode */
/* Description : */
/* --------------------------------------------------------------------- */
BOOLEAN
XGI_DisableChISLCD(PVB_DEVICE_INFO pVBInfo)
{
    USHORT tempbx, tempah;

    tempbx = pVBInfo->SetFlag & (DisableChA | DisableChB);
    tempah = ~((USHORT) XGI_GetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x2E));

    if (tempbx & (EnableChA | DisableChA)) {
        if (!(tempah & 0x08))   /* Chk LCDA Mode */
            return FALSE;
    }

    if (!(tempbx & (EnableChB | DisableChB)))
        return FALSE;

    if (tempah & 0x01)          /* Chk LCDB Mode */
        return TRUE;

    return FALSE;
}


/* --------------------------------------------------------------------- */
/* Function : XGI_EnableChISLCD */
/* Input : */
/* Output : 0 -> Not LCD mode */
/* Description : */
/* --------------------------------------------------------------------- */
BOOLEAN
XGI_EnableChISLCD(PVB_DEVICE_INFO pVBInfo)
{
    USHORT tempbx, tempah;


    tempbx = pVBInfo->SetFlag & (EnableChA | EnableChB);
    tempah = ~((USHORT) XGI_GetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x2E));

    if (tempbx & (EnableChA | DisableChA)) {
        if (!(tempah & 0x08))   /* Chk LCDA Mode */
            return FALSE;
    }

    if (!(tempbx & (EnableChB | DisableChB)))
        return FALSE;

    if (tempah & 0x01)          /* Chk LCDB Mode */
        return TRUE;

    return FALSE;
}


/* --------------------------------------------------------------------- */
/* Function : XGI_GetLCDCapPtr */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
USHORT
XGI_GetLCDCapPtr(PVB_DEVICE_INFO pVBInfo)
{
    UCHAR tempal, tempah, tempbl, i;

    tempah = XGI_GetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x36);
    tempal = tempah & 0x0F;
    tempah = tempah & 0xF0;
    i = 0;
    tempbl = pVBInfo->LCDCapList[i].LCD_ID;

    while (tempbl != 0xFF) {
        if (tempbl & 0x80) {    /* OEMUtil */
            tempal = tempah;
            tempbl = tempbl & ~(0x80);
        }

        if (tempal == tempbl)
            break;

        i++;

        tempbl = pVBInfo->LCDCapList[i].LCD_ID;
    }

    return i;
}


/* --------------------------------------------------------------------- */
/* Function : XGI_GetLCDCapPtr1 */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
USHORT
XGI_GetLCDCapPtr1(PVB_DEVICE_INFO pVBInfo)
{
    USHORT tempah, tempal, tempbl, i;

    tempal = pVBInfo->LCDResInfo;
    tempah = pVBInfo->LCDTypeInfo;

    i = 0;
    tempbl = pVBInfo->LCDCapList[i].LCD_ID;

    while (tempbl != 0xFF) {
        if ((tempbl & 0x80) && (tempbl != 0x80)) {
            tempal = tempah;
            tempbl &= ~0x80;
        }

        if (tempal == tempbl)
            break;

        i++;
        tempbl = pVBInfo->LCDCapList[i].LCD_ID;
    }

    if (tempbl == 0xFF) {
        pVBInfo->LCDResInfo = Panel1024x768;
        pVBInfo->LCDTypeInfo = 0;
        i = 0;
    }

    return i;
}


/* --------------------------------------------------------------------- */
/* Function : XGI_GetLCDSync */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_GetLCDSync(USHORT * HSyncWidth, USHORT * VSyncWidth,
               PVB_DEVICE_INFO pVBInfo)
{
    USHORT Index;

    Index = XGI_GetLCDCapPtr(pVBInfo);
    *HSyncWidth = pVBInfo->LCDCapList[Index].LCD_HSyncWidth;
    *VSyncWidth = pVBInfo->LCDCapList[Index].LCD_VSyncWidth;

    return;
}



/* --------------------------------------------------------------------- */
/* Function : XGI_EnableBridge */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_EnableBridge(PXGI_HW_DEVICE_INFO HwDeviceExtension,
                 PVB_DEVICE_INFO pVBInfo)
{
#ifndef LINUX_XF86
    USHORT tempax;
#endif
    USHORT tempbl, tempah;

    if (pVBInfo->SetFlag == Win9xDOSMode) {
        if (pVBInfo->
            VBType & (VB_XGI301B | VB_XGI302B | VB_XGI301LV | VB_XGI302LV |
                      VB_XGI301C)) {
            XGI_DisplayOn(pVBInfo);
            return;
        }
        else                    /* LVDS or CH7017 */
            return;
    }


    if (HwDeviceExtension->jChipType < XG40) {
        if (!XGI_DisableChISLCD(pVBInfo)) {
            if ((XGI_EnableChISLCD(pVBInfo))
                || (pVBInfo->VBInfo & (SetCRT2ToLCD | SetCRT2ToLCDA))) {
                if (pVBInfo->LCDInfo & SetPWDEnable) {
                    XGI_EnablePWD(pVBInfo);
                }
                else {
                    pVBInfo->LCDInfo &= (~SetPWDEnable);
                    if (pVBInfo->
                        VBType & (VB_XGI301LV | VB_XGI302LV | VB_XGI301C)) {
                        tempbl = 0xFD;
                        tempah = 0x02;
                    }
                    else {
                        tempbl = 0xFB;
                        tempah = 0x00;
                    }

                    XGI_SetPanelPower(tempah, tempbl, pVBInfo);
                    XGI_SetPanelDelay(1, pVBInfo);
                }
            }
        }
    }                           /* Not 340 */



    if (pVBInfo->
        VBType & (VB_XGI301B | VB_XGI302B | VB_XGI301LV | VB_XGI302LV |
                  VB_XGI301C)) {
        if (!(pVBInfo->SetFlag & DisableChA)) {
            if (pVBInfo->SetFlag & EnableChA) {
                XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x1E, 0x20);      /* Power on */
            }
            else {
                if (pVBInfo->VBInfo & SetCRT2ToDualEdge) {      /* SetCRT2ToLCDA ) */
                    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x1E, 0x20);  /* Power on */
                }
            }
        }

        if (!(pVBInfo->SetFlag & DisableChB)) {
            if ((pVBInfo->SetFlag & EnableChB)
                || (pVBInfo->
                    VBInfo & (SetCRT2ToLCD | SetCRT2ToTV | SetCRT2ToRAMDAC)))
            {
                tempah =
                    (UCHAR) XGI_GetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x32);
                tempah &= 0xDF;
                if (pVBInfo->VBInfo & SetInSlaveMode) {
                    if (!(pVBInfo->VBInfo & SetCRT2ToRAMDAC))
                        tempah |= 0x20;
                }
                XGI_SetReg((XGIIOADDRESS) pVBInfo->P3c4, 0x32, tempah);
                XGI_SetRegOR((XGIIOADDRESS) pVBInfo->P3c4, 0x1E, 
			     SR1E_ENABLE_CRT2);


                tempah =
                    (UCHAR) XGI_GetReg((XGIIOADDRESS) pVBInfo->Part1Port,
                                       0x2E);

                if (!(tempah & 0x80))
                    XGI_SetRegOR((XGIIOADDRESS) pVBInfo->Part1Port, 0x2E, 0x80);        /* BVBDOENABLE = 1 */

                XGI_SetRegAND((XGIIOADDRESS) pVBInfo->Part1Port, 0x00, 0x7F);   /* BScreenOFF = 0 */
            }
        }

        if ((pVBInfo->SetFlag & (EnableChA | EnableChB))
            || (!(pVBInfo->VBInfo & DisableCRT2Display))) {
            XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part2Port, 0x00, ~0xE0, 0x20);      /* shampoo 0129 */
            if (pVBInfo->VBType & (VB_XGI302LV | VB_XGI301C)) {
                if (!XGI_DisableChISLCD(pVBInfo)) {
                    if (XGI_EnableChISLCD(pVBInfo)
                        || (pVBInfo->VBInfo & (SetCRT2ToLCD | SetCRT2ToLCDA)))
                        XGI_SetRegAND((XGIIOADDRESS) pVBInfo->Part4Port, 0x2A, 0x7F);   /* LVDS PLL power on */
                }
                XGI_SetRegAND((XGIIOADDRESS) pVBInfo->Part4Port, 0x30, 0x7F);   /* LVDS Driver power on */
            }
        }

        tempah = 0x00;

        if (!(pVBInfo->VBInfo & DisableCRT2Display)) {
            tempah = 0xc0;

            if (!(pVBInfo->VBInfo & SetSimuScanMode)) {
                if (pVBInfo->VBInfo & SetCRT2ToLCDA) {
                    if (pVBInfo->VBInfo & SetCRT2ToDualEdge) {
                        tempah = tempah & 0x40;
                        if (pVBInfo->VBInfo & SetCRT2ToLCDA)
                            tempah = tempah ^ 0xC0;

                        if (pVBInfo->SetFlag & DisableChB)
                            tempah &= 0xBF;

                        if (pVBInfo->SetFlag & DisableChA)
                            tempah &= 0x7F;

                        if (pVBInfo->SetFlag & EnableChB)
                            tempah |= 0x40;

                        if (pVBInfo->SetFlag & EnableChA)
                            tempah |= 0x80;
                    }
                }
            }
        }

        XGI_SetRegOR((XGIIOADDRESS) pVBInfo->Part4Port, 0x1F, tempah);  /* EnablePart4_1F */

        if (pVBInfo->SetFlag & Win9xDOSMode) {
            XGI_DisplayOn(pVBInfo);
            return;
        }

        if (!(pVBInfo->SetFlag & DisableChA)) {
            XGI_VBLongWait(pVBInfo);
            if (!(pVBInfo->SetFlag & GatingCRT)) {
                XGI_DisableGatingCRT(HwDeviceExtension, pVBInfo);
                XGI_DisplayOn(pVBInfo);
                XGI_VBLongWait(pVBInfo);
            }
        }
    }                           /* 301 */
    else {                      /* LVDS */

        if (pVBInfo->VBInfo & (SetCRT2ToTV | SetCRT2ToLCD | SetCRT2ToLCDA))
            XGI_SetRegOR((XGIIOADDRESS) pVBInfo->Part1Port, 0x1E, 0x20);        /* enable CRT2 */



        tempah = (UCHAR) XGI_GetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x2E);
        if (!(tempah & 0x80))
            XGI_SetRegOR((XGIIOADDRESS) pVBInfo->Part1Port, 0x2E, 0x80);        /* BVBDOENABLE = 1 */

        XGI_SetRegAND((XGIIOADDRESS) pVBInfo->Part1Port, 0x00, 0x7F);
        XGI_DisplayOn(pVBInfo);
    }                           /* End of VB */


    if (HwDeviceExtension->jChipType < XG40) {
        if (!XGI_EnableChISLCD(pVBInfo)) {
            if (pVBInfo->VBInfo & (SetCRT2ToLCD | SetCRT2ToLCDA)) {
                if (XGI_BacklightByDrv(pVBInfo))
                    return;
            }
            else
                return;
        }

        if (pVBInfo->LCDInfo & SetPWDEnable) {
            XGI_FirePWDEnable(pVBInfo);
            return;
        }

        XGI_SetPanelDelay(2, pVBInfo);

        if (pVBInfo->VBType & (VB_XGI301LV | VB_XGI302LV | VB_XGI301C)) {
            tempah = 0x01;
            tempbl = 0xFE;      /* turn on backlght */
        }
        else {
            tempbl = 0xF7;
            tempah = 0x00;
        }
        XGI_SetPanelPower(tempah, tempbl, pVBInfo);
    }
}


/* --------------------------------------------------------------------- */
/* Function : XGI_DisableBridge */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_DisableBridge(PXGI_HW_DEVICE_INFO HwDeviceExtension,
                  PVB_DEVICE_INFO pVBInfo)
{
    USHORT tempax, tempbx, tempah = 0, tempbl = 0;

    if (pVBInfo->SetFlag == Win9xDOSMode)
        return;


    if (HwDeviceExtension->jChipType < XG40) {
        if ((!(pVBInfo->VBInfo & (SetCRT2ToLCD | SetCRT2ToLCDA)))
            || (XGI_DisableChISLCD(pVBInfo))) {
            if (!XGI_IsLCDON(pVBInfo)) {
                if (pVBInfo->LCDInfo & SetPWDEnable)
                    XGI_EnablePWD(pVBInfo);
                else {
                    pVBInfo->LCDInfo &= ~SetPWDEnable;
                    XGI_DisablePWD(pVBInfo);
                    if (pVBInfo->
                        VBType & (VB_XGI301LV | VB_XGI302LV | VB_XGI301C)) {
                        tempbx = 0xFE;  /* not 01h */
                        tempax = 0;
                    }
                    else {
                        tempbx = 0xF7;  /* not 08h */
                        tempax = 0x08;
                    }
                    XGI_SetPanelPower(tempax, tempbx, pVBInfo);
                    XGI_SetPanelDelay(3, pVBInfo);
                }
            }                   /* end if(!XGI_IsLCDON(pVBInfo)) */
        }
    }


    if (pVBInfo->
        VBType & (VB_XGI301B | VB_XGI302B | VB_XGI301LV | VB_XGI302LV |
                  VB_XGI301C)) {
        tempah = 0x3F;
        if (!(pVBInfo->VBInfo & (DisableCRT2Display | SetSimuScanMode))) {
            if (pVBInfo->VBInfo & SetCRT2ToLCDA) {
                if (pVBInfo->VBInfo & SetCRT2ToDualEdge) {
                    tempah = 0x7F;      /* Disable Channel A */
                    if (!(pVBInfo->VBInfo & SetCRT2ToLCDA))
                        tempah = 0xBF;  /* Disable Channel B */

                    if (pVBInfo->SetFlag & DisableChB)
                        tempah &= 0xBF; /* force to disable Cahnnel */

                    if (pVBInfo->SetFlag & DisableChA)
                        tempah &= 0x7F; /* Force to disable Channel B */
                }
            }
        }

        XGI_SetRegAND((XGIIOADDRESS) pVBInfo->Part4Port, 0x1F, tempah); /* disable part4_1f */

        if (pVBInfo->VBType & (VB_XGI302LV | VB_XGI301C)) {
            if (((pVBInfo->VBInfo & (SetCRT2ToLCD | SetCRT2ToLCDA)))
                || (XGI_DisableChISLCD(pVBInfo)) || (XGI_IsLCDON(pVBInfo)))
                XGI_SetRegOR((XGIIOADDRESS) pVBInfo->Part4Port, 0x30, 0x80);    /* LVDS Driver power down */
        }

        if ((pVBInfo->SetFlag & DisableChA)
            || (pVBInfo->
                VBInfo & (DisableCRT2Display | SetCRT2ToLCDA |
                          SetSimuScanMode))) {
            if (pVBInfo->SetFlag & GatingCRT)
                XGI_EnableGatingCRT(HwDeviceExtension, pVBInfo);
            XGI_DisplayOff(pVBInfo);
        }

        if (pVBInfo->VBInfo & SetCRT2ToLCDA) {
            if ((pVBInfo->SetFlag & DisableChA)
                || (pVBInfo->VBInfo & SetCRT2ToLCDA))
                XGI_SetRegAND((XGIIOADDRESS) pVBInfo->Part1Port, 0x1e, 0xdf);   /* Power down */
        }

        XGI_SetRegAND((XGIIOADDRESS) pVBInfo->P3c4, 0x32, 0xdf);        /* disable TV as primary VGA swap */

        if ((pVBInfo->VBInfo & (SetSimuScanMode | SetCRT2ToDualEdge)))
            XGI_SetRegAND((XGIIOADDRESS) pVBInfo->Part2Port, 0x00, 0xdf);

        if ((pVBInfo->SetFlag & DisableChB)
            || (pVBInfo->VBInfo & (DisableCRT2Display | SetSimuScanMode))
            || ((!(pVBInfo->VBInfo & SetCRT2ToLCDA))
                && (pVBInfo->
                    VBInfo & (SetCRT2ToRAMDAC | SetCRT2ToLCD | SetCRT2ToTV))))
            XGI_SetRegOR((XGIIOADDRESS) pVBInfo->Part1Port, 0x00, 0x80);        /* BScreenOff=1 */

        if ((pVBInfo->SetFlag & DisableChB)
            || (pVBInfo->VBInfo & (DisableCRT2Display | SetSimuScanMode))
            || (!(pVBInfo->VBInfo & SetCRT2ToLCDA))
            || (pVBInfo->
                VBInfo & (SetCRT2ToRAMDAC | SetCRT2ToLCD | SetCRT2ToTV))) {
            tempah = XGI_GetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x00);       /* save Part1 index 0 */
            XGI_SetRegOR((XGIIOADDRESS) pVBInfo->Part1Port, 0x00, 0x10);        /* BTDAC = 1, avoid VB reset */
            XGI_SetRegAND((XGIIOADDRESS) pVBInfo->Part1Port, 0x1E, 0xDF);       /* disable CRT2 */
            XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x00, tempah);        /* restore Part1 index 0 */
        }
    }
    else {                      /* {301} */

        if (pVBInfo->VBInfo & (SetCRT2ToLCD | SetCRT2ToTV)) {
            XGI_SetRegOR((XGIIOADDRESS) pVBInfo->Part1Port, 0x00, 0x80);        /* BScreenOff=1 */
            XGI_SetRegAND((XGIIOADDRESS) pVBInfo->Part1Port, 0x1E, 0xDF);       /* Disable CRT2 */
            XGI_SetRegAND((XGIIOADDRESS) pVBInfo->P3c4, 0x32, 0xDF);    /* Disable TV asPrimary VGA swap */
        }

        if (pVBInfo->
            VBInfo & (DisableCRT2Display | SetCRT2ToLCDA | SetSimuScanMode))
            XGI_DisplayOff(pVBInfo);
    }




    if (HwDeviceExtension->jChipType < XG40) {
        if (!(pVBInfo->VBInfo & (SetCRT2ToLCD | SetCRT2ToLCDA))
            || (XGI_DisableChISLCD(pVBInfo)) || (XGI_IsLCDON(pVBInfo))) {
            if (pVBInfo->LCDInfo & SetPWDEnable) {
                if (pVBInfo->LCDInfo & SetPWDEnable)
                    XGI_BacklightByDrv(pVBInfo);
                else {
                    XGI_SetPanelDelay(4, pVBInfo);
                    if (pVBInfo->VBType & VB_XGI301LV) {
                        tempbl = 0xFD;
                        tempah = 0x00;
                    }
                    else {
                        tempbl = 0xFB;
                        tempah = 0x04;
                    }
                }
            }
            XGI_SetPanelPower(tempah, tempbl, pVBInfo);
        }
    }
}


/* --------------------------------------------------------------------- */
/* Function : XGI_GetTVPtrIndex */
/* Input : */
/* Output : */
/* Description : bx 0 : ExtNTSC */
/* 1 : StNTSC */
/* 2 : ExtPAL */
/* 3 : StPAL */
/* 4 : ExtHiTV */
/* 5 : StHiTV */
/* 6 : Ext525i */
/* 7 : St525i */
/* 8 : Ext525p */
/* 9 : St525p */
/* A : Ext750p */
/* B : St750p */
/* --------------------------------------------------------------------- */
USHORT
XGI_GetTVPtrIndex(PVB_DEVICE_INFO pVBInfo)
{
    USHORT tempbx = 0;

    if (pVBInfo->TVInfo & SetPALTV)
        tempbx = 2;
    if (pVBInfo->TVInfo & SetYPbPrMode1080i)
        tempbx = 4;
    if (pVBInfo->TVInfo & SetYPbPrMode525i)
        tempbx = 6;
    if (pVBInfo->TVInfo & SetYPbPrMode525p)
        tempbx = 8;
    if (pVBInfo->TVInfo & SetYPbPrMode750p)
        tempbx = 10;
    if (pVBInfo->TVInfo & TVSimuMode)
        tempbx++;

    return tempbx;
}


/* --------------------------------------------------------------------- */
/* Function : XGI_OEM310Setting */
/* Input : */
/* Output : */
/* Description : Customized Param. for 301 */
/* --------------------------------------------------------------------- */
void
XGI_OEM310Setting(USHORT ModeNo, USHORT ModeIdIndex, PVB_DEVICE_INFO pVBInfo)
{
    if (pVBInfo->SetFlag & Win9xDOSMode)
        return;

    /* GetPart1IO(); */
    XGI_SetDelayComp(pVBInfo);

    if (pVBInfo->VBInfo & (SetCRT2ToLCD | SetCRT2ToLCDA))
        XGI_SetLCDCap(pVBInfo);

    if (pVBInfo->VBInfo & SetCRT2ToTV) {
        /* GetPart2IO() */
        XGI_SetPhaseIncr(pVBInfo);
        XGI_SetYFilter(ModeNo, ModeIdIndex, pVBInfo);
        XGI_SetAntiFlicker(ModeNo, ModeIdIndex, pVBInfo);

        if (pVBInfo->VBType & VB_XGI301)
            XGI_SetEdgeEnhance(ModeNo, ModeIdIndex, pVBInfo);
    }
}


/* --------------------------------------------------------------------- */
/* Function : XGI_SetDelayComp */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_SetDelayComp(PVB_DEVICE_INFO pVBInfo)
{
    USHORT index;

    UCHAR tempah, tempbl, tempbh;
#ifndef LINUX_XF86
    UCHAR temp;
#endif

    if (pVBInfo->
        VBType & (VB_XGI301B | VB_XGI302B | VB_XGI301LV | VB_XGI302LV |
                  VB_XGI301C)) {
        if (pVBInfo->
            VBInfo & (SetCRT2ToLCD | SetCRT2ToLCDA | SetCRT2ToTV |
                      SetCRT2ToRAMDAC)) {
            tempbl = 0;
            tempbh = 0;

            index = XGI_GetTVPtrIndex(pVBInfo); /* Get TV Delay */
            tempbl = pVBInfo->XGI_TVDelayList[index];

            if (pVBInfo->
                VBType & (VB_XGI301B | VB_XGI302B | VB_XGI301LV | VB_XGI302LV
                          | VB_XGI301C))
                tempbl = pVBInfo->XGI_TVDelayList2[index];

            if (pVBInfo->VBInfo & SetCRT2ToDualEdge)
                tempbl = tempbl >> 4;
/*
            if ( pVBInfo->VBInfo & SetCRT2ToRAMDAC )
                tempbl = CRT2Delay1 ;			// Get CRT2 Delay

            if ( pVBInfo->VBType & ( VB_XGI301B | VB_XGI302B | VB_XGI301LV | VB_XGI302LV | VB_XGI301C ) )
                tempbl = CRT2Delay2 ;
*/
            if (pVBInfo->VBInfo & (SetCRT2ToLCD | SetCRT2ToLCDA)) {
                index = XGI_GetLCDCapPtr(pVBInfo);      /* Get LCD Delay */
                tempbh = pVBInfo->LCDCapList[index].LCD_DelayCompensation;

                if (!(pVBInfo->VBInfo & SetCRT2ToLCDA))
                    tempbl = tempbh;
            }

            tempbl &= 0x0F;
            tempbh &= 0xF0;
            tempah = XGI_GetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x2D);

            if (pVBInfo->VBInfo & (SetCRT2ToRAMDAC | SetCRT2ToLCD | SetCRT2ToTV)) {     /* Channel B */
                tempah &= 0xF0;
                tempah |= tempbl;
            }

            if (pVBInfo->VBInfo & SetCRT2ToLCDA) {      /* Channel A */
                tempah &= 0x0F;
                tempah |= tempbh;
            }
            XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x2D, tempah);
        }
    }
}


/* --------------------------------------------------------------------- */
/* Function : XGI_SetLCDCap */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_SetLCDCap(PVB_DEVICE_INFO pVBInfo)
{
    USHORT tempcx;

    tempcx = pVBInfo->LCDCapList[XGI_GetLCDCapPtr(pVBInfo)].LCD_Capability;

    if (pVBInfo->
        VBType & (VB_XGI301B | VB_XGI302B | VB_XGI301LV | VB_XGI302LV |
                  VB_XGI301C)) {
        if (pVBInfo->VBType & (VB_XGI301LV | VB_XGI302LV | VB_XGI301C)) {       /* 301LV/302LV only */
            /* Set 301LV Capability */
            XGI_SetReg((XGIIOADDRESS) pVBInfo->Part4Port, 0x24,
                       (UCHAR) (tempcx & 0x1F));
        }
        /* VB Driving */
        XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part4Port, 0x0D,
                        ~((EnableVBCLKDRVLOW | EnablePLLSPLOW) >> 8),
                        (USHORT) ((tempcx &
                                   (EnableVBCLKDRVLOW | EnablePLLSPLOW)) >>
                                  8));
    }

    if (pVBInfo->
        VBType & (VB_XGI301B | VB_XGI302B | VB_XGI301LV | VB_XGI302LV |
                  VB_XGI301C)) {
        if (pVBInfo->VBInfo & SetCRT2ToLCD)
            XGI_SetLCDCap_B(tempcx, pVBInfo);
        else if (pVBInfo->VBInfo & SetCRT2ToLCDA)
            XGI_SetLCDCap_A(tempcx, pVBInfo);

        if (pVBInfo->VBType & (VB_XGI302LV | VB_XGI301C)) {
            if (tempcx & EnableSpectrum)
                SetSpectrum(pVBInfo);
        }
    }
    else                        /* LVDS,CH7017 */
        XGI_SetLCDCap_A(tempcx, pVBInfo);
}


/* --------------------------------------------------------------------- */
/* Function : XGI_SetLCDCap_A */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_SetLCDCap_A(USHORT tempcx, PVB_DEVICE_INFO pVBInfo)
{
    USHORT temp;

    temp = XGI_GetReg((XGIIOADDRESS) pVBInfo->P3d4, 0x37);

    if (temp & LCDRGB18Bit) {
        XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part1Port, 0x19, 0x0F, (USHORT) (0x20 | (tempcx & 0x00C0)));    /* Enable Dither */
        XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part1Port, 0x1A, 0x7F, 0x80);
    }
    else {
        XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part1Port, 0x19, 0x0F,
                        (USHORT) (0x30 | (tempcx & 0x00C0)));
        XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part1Port, 0x1A, 0x7F, 0x00);
    }

/*
    if ( tempcx & EnableLCD24bpp )	// 24bits
    {
        XGI_SetRegANDOR((XGIIOADDRESS)pVBInfo->Part1Port,0x19, 0x0F,(USHORT)(0x30|(tempcx&0x00C0)) );
        XGI_SetRegANDOR((XGIIOADDRESS)pVBInfo->Part1Port,0x1A,0x7F,0x00);
    }
    else
    {
        XGI_SetRegANDOR((XGIIOADDRESS)pVBInfo->Part1Port,0x19, 0x0F,(USHORT)(0x20|(tempcx&0x00C0)) );//Enable Dither
        XGI_SetRegANDOR((XGIIOADDRESS)pVBInfo->Part1Port,0x1A,0x7F,0x80);
    }
*/
}


/* --------------------------------------------------------------------- */
/* Function : XGI_SetLCDCap_B */
/* Input : cx -> LCD Capability */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_SetLCDCap_B(USHORT tempcx, PVB_DEVICE_INFO pVBInfo)
{
    if (tempcx & EnableLCD24bpp)        /* 24bits */
        XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part2Port, 0x1A, 0xE0,
                        (USHORT) (((tempcx & 0x00ff) >> 6) | 0x0c));
    else
        XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part2Port, 0x1A, 0xE0, (USHORT) (((tempcx & 0x00ff) >> 6) | 0x18));     /* Enable Dither */
}


/* --------------------------------------------------------------------- */
/* Function : SetSpectrum */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
SetSpectrum(PVB_DEVICE_INFO pVBInfo)
{
    USHORT index;

    index = XGI_GetLCDCapPtr(pVBInfo);

    XGI_SetRegAND((XGIIOADDRESS) pVBInfo->Part4Port, 0x30, 0x8F);       /* disable down spectrum D[4] */
    XGI_WaitEndRetrace(pVBInfo->RelIO);
    XGI_SetRegOR((XGIIOADDRESS) pVBInfo->Part4Port, 0x30, 0x20);        /* reset spectrum */
    XGI_WaitEndRetrace(pVBInfo->RelIO);

    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part4Port, 0x31,
               pVBInfo->LCDCapList[index].Spectrum_31);
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part4Port, 0x32,
               pVBInfo->LCDCapList[index].Spectrum_32);
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part4Port, 0x33,
               pVBInfo->LCDCapList[index].Spectrum_33);
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part4Port, 0x34,
               pVBInfo->LCDCapList[index].Spectrum_34);
    XGI_WaitEndRetrace(pVBInfo->RelIO);
    XGI_SetRegOR((XGIIOADDRESS) pVBInfo->Part4Port, 0x30, 0x40);        /* enable spectrum */
}


/* --------------------------------------------------------------------- */
/* Function : XGI_SetAntiFlicker */
/* Input : */
/* Output : */
/* Description : Set TV Customized Param. */
/* --------------------------------------------------------------------- */
void
XGI_SetAntiFlicker(USHORT ModeNo, USHORT ModeIdIndex, PVB_DEVICE_INFO pVBInfo)
{
    USHORT tempbx, index;

    UCHAR tempah;

    if (pVBInfo->TVInfo & (SetYPbPrMode525p | SetYPbPrMode750p))
        return;

    tempbx = XGI_GetTVPtrIndex(pVBInfo);
    tempbx &= 0xFE;

    if (ModeNo <= 0x13) {
        index = pVBInfo->SModeIDTable[ModeIdIndex].VB_StTVFlickerIndex;
    }
    else {
        index = pVBInfo->EModeIDTable[ModeIdIndex].VB_ExtTVFlickerIndex;
    }

    tempbx += index;
    tempah = TVAntiFlickList[tempbx];
    tempah = tempah << 4;

    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part2Port, 0x0A, 0x8F, tempah);
}


/* --------------------------------------------------------------------- */
/* Function : XGI_SetEdgeEnhance */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_SetEdgeEnhance(USHORT ModeNo, USHORT ModeIdIndex, PVB_DEVICE_INFO pVBInfo)
{
    USHORT tempbx, index;

    UCHAR tempah;


    tempbx = XGI_GetTVPtrIndex(pVBInfo);
    tempbx &= 0xFE;

    if (ModeNo <= 0x13) {
        index = pVBInfo->SModeIDTable[ModeIdIndex].VB_StTVEdgeIndex;
    }
    else {
        index = pVBInfo->EModeIDTable[ModeIdIndex].VB_ExtTVEdgeIndex;
    }

    tempbx += index;
    tempah = TVEdgeList[tempbx];
    tempah = tempah << 5;

    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part2Port, 0x3A, 0x1F, tempah);
}


/* --------------------------------------------------------------------- */
/* Function : XGI_SetPhaseIncr */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_SetPhaseIncr(PVB_DEVICE_INFO pVBInfo)
{
    USHORT tempbx;

    UCHAR tempcl, tempch;

    ULONG tempData;

    XGI_GetTVPtrIndex2(&tempbx, &tempcl, &tempch, pVBInfo);     /* bx, cl, ch */
    tempData = TVPhaseList[tempbx];

    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x31,
               (USHORT) (tempData & 0x000000FF));
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x32,
               (USHORT) ((tempData & 0x0000FF00) >> 8));
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x33,
               (USHORT) ((tempData & 0x00FF0000) >> 16));
    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x34,
               (USHORT) ((tempData & 0xFF000000) >> 24));
}


/* --------------------------------------------------------------------- */
/* Function : XGI_SetYFilter */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_SetYFilter(USHORT ModeNo, USHORT ModeIdIndex, PVB_DEVICE_INFO pVBInfo)
{
    USHORT tempbx, index;

    UCHAR tempcl, tempch, tempal;
    const UCHAR *filterPtr;

    XGI_GetTVPtrIndex2(&tempbx, &tempcl, &tempch, pVBInfo);     /* bx, cl, ch */

    switch (tempbx) {
    case 0x00:
    case 0x04:
        filterPtr = NTSCYFilter1;
        break;

    case 0x01:
        filterPtr = PALYFilter1;
        break;

    case 0x02:
    case 0x05:
    case 0x0D:
        filterPtr = PALMYFilter1;
        break;

    case 0x03:
        filterPtr = PALNYFilter1;
        break;

    case 0x08:
    case 0x0C:
        filterPtr = NTSCYFilter2;
        break;

    case 0x0A:
        filterPtr = PALMYFilter2;
        break;

    case 0x0B:
        filterPtr = PALNYFilter2;
        break;

    case 0x09:
        filterPtr = PALYFilter2;
        break;

    default:
        return;
    }

    if (ModeNo <= 0x13)
        tempal = pVBInfo->SModeIDTable[ModeIdIndex].VB_StTVYFilterIndex;
    else
        tempal = pVBInfo->EModeIDTable[ModeIdIndex].VB_ExtTVYFilterIndex;

    if (tempcl == 0)
        index = tempal * 4;
    else
        index = tempal * 7;

    if ((tempcl == 0) && (tempch == 1)) {
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x35, 0);
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x36, 0);
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x37, 0);
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x38,
                   filterPtr[index++]);
    }
    else {
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x35,
                   filterPtr[index++]);
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x36,
                   filterPtr[index++]);
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x37,
                   filterPtr[index++]);
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x38,
                   filterPtr[index++]);
    }

    if (pVBInfo->
        VBType & (VB_XGI301B | VB_XGI302B | VB_XGI301LV | VB_XGI302LV |
                  VB_XGI301C)) {
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x48,
                   filterPtr[index++]);
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x49,
                   filterPtr[index++]);
        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part2Port, 0x4A,
                   filterPtr[index++]);
    }
}


/* --------------------------------------------------------------------- */
/* Function : XGI_GetTVPtrIndex2 */
/* Input : */
/* Output : bx 0 : NTSC */
/* 1 : PAL */
/* 2 : PALM */
/* 3 : PALN */
/* 4 : NTSC1024x768 */
/* 5 : PAL-M 1024x768 */
/* 6-7: reserved */
/* cl 0 : YFilter1 */
/* 1 : YFilter2 */
/* ch 0 : 301A */
/* 1 : 301B/302B/301LV/302LV */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_GetTVPtrIndex2(USHORT * tempbx, UCHAR * tempcl, UCHAR * tempch,
                   PVB_DEVICE_INFO pVBInfo)
{
    *tempbx = 0;
    *tempcl = 0;
    *tempch = 0;

    if (pVBInfo->TVInfo & SetPALTV)
        *tempbx = 1;

    if (pVBInfo->TVInfo & SetPALMTV)
        *tempbx = 2;

    if (pVBInfo->TVInfo & SetPALNTV)
        *tempbx = 3;

    if (pVBInfo->TVInfo & NTSC1024x768) {
        *tempbx = 4;
        if (pVBInfo->TVInfo & SetPALMTV)
            *tempbx = 5;
    }

    if (pVBInfo->
        VBType & (VB_XGI301B | VB_XGI302B | VB_XGI301LV | VB_XGI302LV |
                  VB_XGI301C)) {
        if ((!(pVBInfo->VBInfo & SetInSlaveMode))
            || (pVBInfo->TVInfo & TVSimuMode)) {
            *tempbx += 8;
            *tempcl += 1;
        }
    }

    if (pVBInfo->
        VBType & (VB_XGI301B | VB_XGI302B | VB_XGI301LV | VB_XGI302LV |
                  VB_XGI301C))
        *tempch++;
}


/* --------------------------------------------------------------------- */
/* Function : XGI_SetCRT2ModeRegs */
/* Input : */
/* Output : */
/* Description : Origin code for crt2group */
/* --------------------------------------------------------------------- */
void
XGI_SetCRT2ModeRegs(USHORT ModeNo, PXGI_HW_DEVICE_INFO HwDeviceExtension,
                    PVB_DEVICE_INFO pVBInfo)
{
#ifndef LINUX_XF86
    USHORT i, j;
#endif
    USHORT tempbl;
    SHORT tempcl;

    UCHAR tempah;

    /* XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port , 0x03 , 0x00 ) ; // fix write part1 index 0 BTDRAM bit Bug */
    tempah = 0;
    if (!(pVBInfo->VBInfo & DisableCRT2Display)) {
        tempah = XGI_GetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x00);
        tempah &= ~0x10;        /* BTRAMDAC */
        tempah |= 0x40;         /* BTRAM */

        if (pVBInfo->VBInfo & (SetCRT2ToRAMDAC | SetCRT2ToTV | SetCRT2ToLCD)) {
            tempah = 0x40;      /* BTDRAM */
            if (ModeNo > 0x13) {
                tempcl = pVBInfo->ModeType;
                tempcl -= ModeVGA;
                if (tempcl >= 0) {
                    tempah = (0x008 >> tempcl); /* BT Color */
                    if (tempah == 0)
                        tempah = 1;
                    tempah |= 0x040;
                }
            }
            if (pVBInfo->VBInfo & SetInSlaveMode)
                tempah ^= 0x50; /* BTDAC */
        }
    }

/*	0210 shampoo
    if ( pVBInfo->VBInfo & DisableCRT2Display )
    {
        tempah = 0 ;
    }

    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port , 0x00 , tempah ) ;
    if ( pVBInfo->VBInfo & ( SetCRT2ToRAMDAC | SetCRT2ToTV | SetCRT2ToLCD ) )
    {
        tempcl = pVBInfo->ModeType ;
        if ( ModeNo > 0x13 )
        {
            tempcl -= ModeVGA ;
            if ( ( tempcl > 0 ) || ( tempcl == 0 ) )
            {
                tempah=(0x008>>tempcl) ;
                if ( tempah == 0 )
                    tempah = 1 ;
                tempah |= 0x040;
            }
        }
        else
        {
            tempah = 0x040 ;
        }

        if ( pVBInfo->VBInfo & SetInSlaveMode )
        {
            tempah = ( tempah ^ 0x050 ) ;
        }
    }
*/

    XGI_SetReg((XGIIOADDRESS) pVBInfo->Part1Port, 0x00, tempah);
    tempah = 0x08;
    tempbl = 0xf0;

    if (pVBInfo->VBInfo & DisableCRT2Display)
        XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part1Port, 0x2e, tempbl,
                        tempah);
    else {
        tempah = 0x00;
        tempbl = 0xff;

        if (pVBInfo->
            VBInfo & (SetCRT2ToRAMDAC | SetCRT2ToTV | SetCRT2ToLCD |
                      SetCRT2ToLCDA)) {
            if ((pVBInfo->VBInfo & SetCRT2ToLCDA)
                && (!(pVBInfo->VBInfo & SetSimuScanMode))) {
                tempbl &= 0xf7;
                tempah |= 0x01;
                XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part1Port, 0x2e,
                                tempbl, tempah);
            }
            else {
                if (pVBInfo->VBInfo & SetCRT2ToLCDA) {
                    tempbl &= 0xf7;
                    tempah |= 0x01;
                }

                if (pVBInfo->
                    VBInfo & (SetCRT2ToRAMDAC | SetCRT2ToTV | SetCRT2ToLCD)) {
                    tempbl &= 0xf8;
                    tempah = 0x01;

                    if (!(pVBInfo->VBInfo & SetInSlaveMode))
                        tempah |= 0x02;

                    if (!(pVBInfo->VBInfo & SetCRT2ToRAMDAC)) {
                        tempah = tempah ^ 0x05;
                        if (!(pVBInfo->VBInfo & SetCRT2ToLCD))
                            tempah = tempah ^ 0x01;
                    }

                    if (!(pVBInfo->VBInfo & SetCRT2ToDualEdge))
                        tempah |= 0x08;
                    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part1Port, 0x2e,
                                    tempbl, tempah);
                }
                else
                    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part1Port, 0x2e,
                                    tempbl, tempah);
            }
        }
        else
            XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part1Port, 0x2e, tempbl,
                            tempah);
    }

    if (pVBInfo->
        VBInfo & (SetCRT2ToRAMDAC | SetCRT2ToTV | SetCRT2ToLCD |
                  SetCRT2ToLCDA)) {
        tempah &= (~0x08);
        if ((pVBInfo->ModeType == ModeVGA)
            && (!(pVBInfo->VBInfo & SetInSlaveMode))) {
            tempah |= 0x010;
        }
        tempah |= 0x080;

        if (pVBInfo->VBInfo & SetCRT2ToTV) {
            /* if ( !( pVBInfo->TVInfo & ( SetYPbPrMode525p | SetYPbPrMode750p ) ) ) */
            /* { */
            tempah |= 0x020;
            if (ModeNo > 0x13) {
                if (pVBInfo->VBInfo & DriverMode)
                    tempah = tempah ^ 0x20;
            }
            /* } */
        }

        XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part4Port, 0x0D, ~0x0BF,
                        tempah);
        tempah = 0;

        if (pVBInfo->LCDInfo & SetLCDDualLink)
            tempah |= 0x40;

        if (pVBInfo->VBInfo & SetCRT2ToTV) {
            /* if ( ( !( pVBInfo->VBInfo & SetCRT2ToHiVisionTV ) ) && ( !( pVBInfo->TVInfo & ( SetYPbPrMode525p | SetYPbPrMode750p ) ) ) ) */
            /* { */
            if (pVBInfo->TVInfo & RPLLDIV2XO)
                tempah |= 0x40;
            /* } */
        }

        if ((pVBInfo->LCDResInfo == Panel1280x1024)
            || (pVBInfo->LCDResInfo == Panel1280x1024x75))
            tempah |= 0x80;

        if (pVBInfo->LCDResInfo == Panel1280x960)
            tempah |= 0x80;

        XGI_SetReg((XGIIOADDRESS) pVBInfo->Part4Port, 0x0C, tempah);
    }

    if (pVBInfo->
        VBType & (VB_XGI301B | VB_XGI302B | VB_XGI301LV | VB_XGI302LV |
                  VB_XGI301C)) {
        tempah = 0;
        tempbl = 0xfb;

        if (pVBInfo->VBInfo & SetCRT2ToDualEdge) {
            tempbl = 0xff;
            if (pVBInfo->VBInfo & SetCRT2ToLCDA)
                tempah |= 0x04; /* shampoo 0129 */
        }

        XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part1Port, 0x13, tempbl,
                        tempah);
        tempah = 0x00;
        tempbl = 0xcf;
        if (!(pVBInfo->VBInfo & DisableCRT2Display)) {
            if (pVBInfo->VBInfo & SetCRT2ToDualEdge)
                tempah |= 0x30;
        }

        XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part1Port, 0x2c, tempbl,
                        tempah);
        tempah = 0;
        tempbl = 0x3f;

        if (!(pVBInfo->VBInfo & DisableCRT2Display)) {
            if (pVBInfo->VBInfo & SetCRT2ToDualEdge)
                tempah |= 0xc0;
        }
        XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part4Port, 0x21, tempbl,
                        tempah);
    }

    tempah = 0;
    tempbl = 0x7f;
    if (!(pVBInfo->VBInfo & SetCRT2ToLCDA)) {
        tempbl = 0xff;
        if (!(pVBInfo->VBInfo & SetCRT2ToDualEdge))
            tempah |= 0x80;
    }

    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part4Port, 0x23, tempbl, tempah);

    if (pVBInfo->VBType & (VB_XGI302LV | VB_XGI301C)) {
        if (pVBInfo->LCDInfo & SetLCDDualLink) {
            XGI_SetRegOR((XGIIOADDRESS) pVBInfo->Part4Port, 0x27, 0x20);
            XGI_SetRegOR((XGIIOADDRESS) pVBInfo->Part4Port, 0x34, 0x10);
        }
    }
}


/* --------------------------------------------------------------------- */
/* Function : XGI_GetRAMDAC2DATA */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_GetRAMDAC2DATA(USHORT ModeNo, USHORT ModeIdIndex,
                   USHORT RefreshRateTableIndex, PVB_DEVICE_INFO pVBInfo)
{
    USHORT tempax, tempbx, temp1, temp2, modeflag = 0, tempcx, CRT1Index;
#ifndef LINUX_XF86
    USHORT temp, ResInfo, DisplayType;
#endif

    pVBInfo->RVBHCMAX = 1;
    pVBInfo->RVBHCFACT = 1;

    if (ModeNo <= 0x13) {
        const USHORT StandTableIndex = XGI_GetModePtr(pVBInfo->SModeIDTable,
                                                      pVBInfo->ModeType,
                                                      ModeNo, ModeIdIndex);

        modeflag = pVBInfo->SModeIDTable[ModeIdIndex].St_ModeFlag;
        tempax = pVBInfo->StandTable[StandTableIndex].CRTC[0];
        tempbx = pVBInfo->StandTable[StandTableIndex].CRTC[6];
        temp1 = pVBInfo->StandTable[StandTableIndex].CRTC[7];
    }
    else {
        modeflag = pVBInfo->EModeIDTable[ModeIdIndex].Ext_ModeFlag;
        CRT1Index = pVBInfo->RefIndex[RefreshRateTableIndex].Ext_CRT1CRTC;
        CRT1Index &= IndexMask;
        temp1 = (USHORT) pVBInfo->XGINEWUB_CRT1Table[CRT1Index].CR[0];
        temp2 = (USHORT) pVBInfo->XGINEWUB_CRT1Table[CRT1Index].CR[5];
        tempax = (temp1 & 0xFF) | ((temp2 & 0x03) << 8);
        tempbx = (USHORT) pVBInfo->XGINEWUB_CRT1Table[CRT1Index].CR[8];
        tempcx = (USHORT) pVBInfo->XGINEWUB_CRT1Table[CRT1Index].CR[14] << 8;
        tempcx &= 0x0100;
        tempcx = tempcx << 2;
        tempbx |= tempcx;
        temp1 = (USHORT) pVBInfo->XGINEWUB_CRT1Table[CRT1Index].CR[9];
    }

    if (temp1 & 0x01)
        tempbx |= 0x0100;

    if (temp1 & 0x20)
        tempbx |= 0x0200;
    tempax += 5;

    if (modeflag & Charx8Dot)
        tempax *= 8;
    else
        tempax *= 9;

    pVBInfo->VGAHT = tempax;
    pVBInfo->HT = tempax;
    tempbx++;
    pVBInfo->VGAVT = tempbx;
    pVBInfo->VT = tempbx;
}



/* --------------------------------------------------------------------- */
/* Function : XGI_GetColorDepth */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
USHORT
XGI_GetColorDepth(USHORT ModeNo, USHORT ModeIdIndex,
                  const VB_DEVICE_INFO *pVBInfo)
{
    USHORT ColorDepth[6] = { 1, 2, 4, 4, 6, 8 };
    SHORT index;
    USHORT modeflag;

    if (ModeNo <= 0x13) {
        modeflag = pVBInfo->SModeIDTable[ModeIdIndex].St_ModeFlag;
    }
    else {
        modeflag = pVBInfo->EModeIDTable[ModeIdIndex].Ext_ModeFlag;
    }

    index = (modeflag & ModeInfoFlag) - ModeEGA;

    if (index < 0)
        index = 0;

    return (ColorDepth[index]);
}



/* --------------------------------------------------------------------- */
/* Function : XGI_UnLockCRT2 */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_UnLockCRT2(PXGI_HW_DEVICE_INFO HwDeviceExtension, PVB_DEVICE_INFO pVBInfo)
{

    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part1Port, 0x2f, 0xFF, 0x01);

}


/* --------------------------------------------------------------------- */
/* Function : XGI_LockCRT2 */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_LockCRT2(PXGI_HW_DEVICE_INFO HwDeviceExtension, PVB_DEVICE_INFO pVBInfo)
{
    XGI_SetRegANDOR((XGIIOADDRESS) pVBInfo->Part1Port, 0x2F, 0xFE, 0x00);
}


/* --------------------------------------------------------------------- */
/* Function : XGINew_EnableCRT2 */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGINew_EnableCRT2(PVB_DEVICE_INFO pVBInfo)
{
    XGI_SetRegOR((XGIIOADDRESS) pVBInfo->P3c4, 0x1E, SR1E_ENABLE_CRT2);
}



/* --------------------------------------------------------------------- */
/* Function : */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGINew_LCD_Wait_Time(UCHAR DelayTime, PVB_DEVICE_INFO pVBInfo)
{
    USHORT i, j;

    ULONG temp, flag;

    flag = 0;

    for (i = 0; i < DelayTime; i++) {
        for (j = 0; j < 66; j++) {
            temp = XGI_GetRegLong((XGIIOADDRESS) 0x61);
            temp &= 0x10;

            if (temp == flag)
                continue;

            flag = temp;
        }
    }
}




/* --------------------------------------------------------------------- */
/* Function : XGI_BridgeIsOn */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
BOOLEAN
XGI_BridgeIsOn(PVB_DEVICE_INFO pVBInfo)
{
    USHORT flag;

    flag = XGI_GetReg((XGIIOADDRESS) pVBInfo->Part4Port, 0x00);
    if ((flag == 1) || (flag == 2))
        return (1);             /* 301b */
    else
        return (0);
}


/* --------------------------------------------------------------------- */
/* Function : XGI_VBLongWait */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
void
XGI_VBLongWait(PVB_DEVICE_INFO pVBInfo)
{
    USHORT tempal, temp, i, j;

    if (!(pVBInfo->VBInfo & SetCRT2ToTV)) {
        temp = 0;
        for (i = 0; i < 3; i++) {
            for (j = 0; j < 100; j++) {
                tempal = XGI_GetRegByte((XGIIOADDRESS) pVBInfo->P3da);
                if (temp & 0x01) {      /* VBWaitMode2 */
                    if ((tempal & 0x08)) {
                        continue;
                    }

                    if (!(tempal & 0x08)) {
                        break;
                    }
                }
                else {          /* VBWaitMode1 */
                    if (!(tempal & 0x08)) {
                        continue;
                    }

                    if ((tempal & 0x08)) {
                        break;
                    }
                }
            }
            temp = temp ^ 0x01;
        }
    }
    else {
        XGI_WaitEndRetrace(pVBInfo->RelIO);
    }
    return;
}




/* --------------------------------------------------------------------- */
/* Function : XGI_GetVGAHT2 */
/* Input : */
/* Output : */
/* Description : */
/* --------------------------------------------------------------------- */
USHORT
XGI_GetVGAHT2(PVB_DEVICE_INFO pVBInfo)
{
    ULONG tempax, tempbx;

    tempbx =
        ((pVBInfo->VGAVT - pVBInfo->VGAVDE) * pVBInfo->RVBHCMAX) & 0xFFFF;
    tempax = (pVBInfo->VT - pVBInfo->VDE) * pVBInfo->RVBHCFACT;
    tempax = (tempax * pVBInfo->HT) / tempbx;

    return ((USHORT) tempax);
}


/**
 * Get magic index into clock table.
 * 
 * \bugs
 * I'm pretty sure the first if-statement is wrong.  It will \b always
 * evaluate to true.
 */
unsigned
XGI_GetVCLK2Ptr(USHORT ModeNo, USHORT ModeIdIndex,
                USHORT RefreshRateTableIndex,
                PVB_DEVICE_INFO pVBInfo)
{
    unsigned VCLKIndex;
    const unsigned modeflag = (ModeNo <= 0x13)
        ? pVBInfo->SModeIDTable[ModeIdIndex].St_ModeFlag
        : pVBInfo->EModeIDTable[ModeIdIndex].Ext_ModeFlag;


    if (((pVBInfo->VBInfo & SetCRT2ToLCD) | SetCRT2ToLCDA)) {   /*301b */
        VCLKIndex = (pVBInfo->LCDResInfo != Panel1024x768)
            ? (VCLK108_2 + 5) : (VCLK65 + 2);
    }
    else {                      /* for TV */
        if (pVBInfo->VBInfo & SetCRT2ToTV) {
            if (pVBInfo->VBInfo & SetCRT2ToHiVisionTV) {
                VCLKIndex = (pVBInfo->SetFlag & RPLLDIV2XO)
                    ? HiTVVCLKDIV2 : HiTVVCLK;

                VCLKIndex += 25;

                if (pVBInfo->SetFlag & TVSimuMode) {
                    VCLKIndex = (modeflag & Charx8Dot)
                        ? HiTVSimuVCLK : HiTVTextVCLK;

                    VCLKIndex += 25;
                }

                if (pVBInfo->VBType & VB_XGI301LV) {
                    switch (pVBInfo->VBExtInfo) {
                    case VB_YPbPr1080i:
                        /* VCLKIndex already set to correct value? */
                        break;
                    case VB_YPbPr750p:
                        VCLKIndex = YPbPr750pVCLK;
                        break;
                    case VB_YPbPr525p:
                        VCLKIndex = YPbPr525pVCLK;
                        break;
                    case VB_YPbPr525i:
                        VCLKIndex = (pVBInfo->SetFlag & RPLLDIV2XO)
                            ? YPbPr525iVCLK_2 : YPbPr525iVCLK;
                        break;
                    }
                }
            }
            else {
                VCLKIndex = (pVBInfo->SetFlag & RPLLDIV2XO)
                    ? TVVCLKDIV2 : TVVCLK;

                VCLKIndex += 25;
            }
        }
        else {                  /* for CRT2 */
            VCLKIndex = XGI_GetRegByte((XGIIOADDRESS) (pVBInfo->P3ca + 0x02));
            VCLKIndex = ((VCLKIndex >> 2) & 0x03);
            if (ModeNo > 0x13) {
                VCLKIndex = 
                    (pVBInfo->RefIndex[RefreshRateTableIndex].Ext_CRTVCLK
                     & IndexMask);
            }
        }
    }

    return VCLKIndex;
}
