/**************************************************************************
 * 
 * Copyright (c) Intel Corp. 2007.
 * All Rights Reserved.
 *
 * Intel funded Tungsten Graphics (http://www.tungstengraphics.com) to
 * develop this driver.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *  * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS, AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE 
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/
/*
 * Authors: Alan Hourihane <alanh@tungstengraphics.com>
 *	    Michel Dänzer <michel@tungstengraphics.com>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "miline.h"

#include "vermilion.h"
#include "vermilion_mbx.h"

#include "xaarop.h"

static void mbxSync(ScrnInfoRec * pScrn);
static void mbxSetupForFillRectSolid(ScrnInfoRec * pScrn, int color,
    int rop, unsigned int planemask);
static void mbxSubsequentFillRectSolid(ScrnInfoRec * pScrn, int x,
    int y, int w, int h);
static void mbxSubsequentScreenToScreenCopy(ScrnInfoRec * pScrn,
    int x1, int y1, int x2, int y2, int w, int h);
static void mbxSetupForScreenToScreenCopy(ScrnInfoRec * pScrn,
    int xdir, int ydir, int rop,
    unsigned int planemask, int transparency_color);

#define MBX_SYNC_MAP_SIZE 4

Bool
VERMILIONAccelInit(ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    VERMILIONPtr pVermilion = VERMILIONPTR(pScrn);
    XAAInfoRecPtr infoPtr;
    BoxRec AvailFBArea;

    switch (pScrn->depth) {
    case 15:
	pVermilion->mbxBpp = MBX2D_SRC_555RGB;
	break;
    case 24:
	pVermilion->mbxBpp = MBX2D_SRC_8888ARGB;
	break;
    default:
	return FALSE;
    }

    pVermilion->accel = infoPtr = XAACreateInfoRec();
    if (!infoPtr)
	return FALSE;

    infoPtr->Flags = PIXMAP_CACHE | OFFSCREEN_PIXMAPS | LINEAR_FRAMEBUFFER;

    infoPtr->Sync = mbxSync;

    infoPtr->SolidFillFlags = NO_PLANEMASK;
    infoPtr->SetupForSolidFill = mbxSetupForFillRectSolid;
    infoPtr->SubsequentSolidFillRect = mbxSubsequentFillRectSolid;

    infoPtr->ScreenToScreenCopyFlags = NO_PLANEMASK;
    infoPtr->SetupForScreenToScreenCopy = mbxSetupForScreenToScreenCopy;
    infoPtr->SubsequentScreenToScreenCopy = mbxSubsequentScreenToScreenCopy;

    AvailFBArea.x1 = 0;
    AvailFBArea.y1 = 0;
    AvailFBArea.x2 = pScrn->displayWidth;
    AvailFBArea.y2 =
	(pVermilion->fbSize - MBX_SYNC_MAP_SIZE) / pVermilion->stride;

    if (AvailFBArea.y2 > 4095)
	AvailFBArea.y2 = 4095;

    xf86InitFBManager(pScreen, &AvailFBArea);

    if (!XAAInit(pScreen, infoPtr))
	return FALSE;

    pVermilion->mbxFBDevAddr = pScrn->memPhysBase;

    /* Reserve DWORD at the end of the framebuffer for mbxSync() */
    pVermilion->mbxSyncDevAddr = pVermilion->mbxFBDevAddr +
	pVermilion->fbSize - MBX_SYNC_MAP_SIZE;
    pVermilion->mbxSyncMap = (CARD32 *) ((char *)pVermilion->fbMap +
	pVermilion->fbSize - MBX_SYNC_MAP_SIZE);

    pVermilion->slavePort = (CARD32 *) ((char *)pVermilion->mbxRegsBase +
	MBX_SP_2D_SYS_PHYS_OFFSET);

    return TRUE;
}

static void
mbxSync(ScrnInfoRec * pScrn)
{
    VERMILIONPtr pVermilion = VERMILIONPTR(pScrn);
    unsigned int sync_val;
    volatile CARD32 *fb_sync_val;
    CARD32 auBltPacket[7];

    fb_sync_val = pVermilion->mbxSyncMap;
    sync_val = ((*fb_sync_val) + 1) % 65536;

    WAITFIFO(7);

    auBltPacket[0] = MBX2D_DST_CTRL_BH | MBX2D_SRC_8888ARGB;
    auBltPacket[1] = pVermilion->mbxSyncDevAddr;
    auBltPacket[2] = MBX2D_BLIT_BH | ROP_P << 8 | ROP_P;
    auBltPacket[3] = sync_val;
    auBltPacket[4] = (0 & 0xffff) << 16 | (0 & 0xffff);
    auBltPacket[5] = (1 & 0xffff) << 16 | (1 & 0xffff);
    auBltPacket[6] = MBX2D_FENCE_BH;

    WRITESLAVEPORTDATA(7);

    while (*fb_sync_val != sync_val) {
#if 0
	ErrorF("WAITING 0x%x 0x%x\n", *fb_sync_val, sync_val);
#endif
	usleep(10);
    }
}

static void
mbxSetupForScreenToScreenCopy(ScrnInfoRec * pScrn,
    int xdir, int ydir, int rop,
    unsigned int planemask, int transparency_color)
{
    VERMILIONPtr pVermilion = VERMILIONPTR(pScrn);
    CARD32 auBltPacket[4];

    pVermilion->transEnable = 0;

    pVermilion->ROP = XAAGetCopyROP(rop);

    pVermilion->dir = MBX2D_TEXTCOPY_TL2BR;
    if (xdir < 0)
	pVermilion->dir |= MBX2D_TEXTCOPY_TR2BL;
    if (ydir < 0)
	pVermilion->dir |= MBX2D_TEXTCOPY_BL2TR;

    if (transparency_color != -1) {
	pVermilion->transEnable = MBX2D_SRCCK_REJECT;

	WAITFIFO(3);

	auBltPacket[0] = MBX2D_CTRL_BH | MBX2D_SRCCK_CTRL;
	if (pScrn->depth == 15)
	    auBltPacket[1] = transparency_color | transparency_color << 16;
	else
	    auBltPacket[1] = transparency_color;
	auBltPacket[2] = 0xffffffff;

	WRITESLAVEPORTDATA(3);
    }

    WAITFIFO(4);

    auBltPacket[0] = MBX2D_SRC_CTRL_BH | MBX2D_SRC_FBMEM | pVermilion->mbxBpp;
    auBltPacket[0] |= pVermilion->stride;
    auBltPacket[1] = pVermilion->mbxFBDevAddr;
    auBltPacket[2] = MBX2D_DST_CTRL_BH | pVermilion->mbxBpp;
    auBltPacket[2] |= pVermilion->stride;
    auBltPacket[3] = pVermilion->mbxFBDevAddr;
    WRITESLAVEPORTDATA(4);
}

static void
mbxSubsequentScreenToScreenCopy(ScrnInfoRec * pScrn, int x1, int y1,
    int x2, int y2, int w, int h)
{
    VERMILIONPtr pVermilion = VERMILIONPTR(pScrn);
    CARD32 auBltPacket[5];

    WAITFIFO(5);

    auBltPacket[0] = MBX2D_SRC_OFF_BH | (x1 & 0xffff) << 14 | (y1 & 0xffff);
    auBltPacket[1] = MBX2D_BLIT_BH | pVermilion->transEnable |
	MBX2D_USE_PAT | pVermilion->dir |
	(pVermilion->ROP & 0xff) << 8 | (pVermilion->ROP & 0xff);
    auBltPacket[2] = (x2 & 0xffff) << 16 | (y2 & 0xffff);
    auBltPacket[3] = ((x2 + w) & 0xffff) << 16 | ((y2 + h) & 0xffff);
    auBltPacket[4] = MBX2D_FENCE_BH;

    WRITESLAVEPORTDATA(5);
}

static void
mbxSetupForFillRectSolid(ScrnInfoRec * pScrn, int color,
    int rop, unsigned int planemask)
{
    VERMILIONPtr pVermilion = VERMILIONPTR(pScrn);
    CARD32 auBltPacket[2];

    pVermilion->ROP = XAAGetPatternROP(rop);

    if (pScrn->depth == 15) {
	color |= 0x8000;
    }

    pVermilion->fillColour = color;

    WAITFIFO(2);

    auBltPacket[0] = MBX2D_DST_CTRL_BH | pVermilion->mbxBpp;
    auBltPacket[0] |= pVermilion->stride;
    auBltPacket[1] = pVermilion->mbxFBDevAddr;
    WRITESLAVEPORTDATA(2);
}

static void
mbxSubsequentFillRectSolid(ScrnInfoRec * pScrn, int x, int y, int w, int h)
{
    VERMILIONPtr pVermilion = VERMILIONPTR(pScrn);
    CARD32 auBltPacket[5];

    WAITFIFO(5);

    auBltPacket[0] = MBX2D_BLIT_BH |
	(pVermilion->ROP & 0xff) << 8 | (pVermilion->ROP & 0xff);
    auBltPacket[1] = pVermilion->fillColour;
    auBltPacket[2] = (x & 0xffff) << 16 | (y & 0xffff);
    auBltPacket[3] = ((x + w) & 0xffff) << 16 | ((y + h) & 0xffff);
    auBltPacket[4] = MBX2D_FENCE_BH;
    WRITESLAVEPORTDATA(5);
}
