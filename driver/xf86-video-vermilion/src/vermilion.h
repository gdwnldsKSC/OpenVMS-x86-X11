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
 * 
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
 * Authors: 
 *   Michel Dänzer <michel-at-tungstengraphics-dot-com>
 *   Thomas Hellström <thomas-at-tungstengraphics-dot-com>
 *   Alan Hourihane <alanh-at-tungstengraphics-dot-com>
 */

#ifndef _VERMILION_H_
#define _VERMILION_H_

/* All drivers should typically include these */
#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86Resources.h"

#include "compiler.h"

/* Drivers for PCI hardware need this */
#include "xf86PciInfo.h"

#include "vgaHW.h"

/* Drivers that need to access the PCI config space directly need this */
#include "xf86Pci.h"

/* VBE/DDC support */
#include "vbe.h"
#include "vbeModes.h"
#include "xf86DDC.h"

/* ShadowFB support */
#include "shadow.h"

/* Int 10 support */
#include "xf86int10.h"

/* bank switching */
#include "mibank.h"

/* Dga definitions */
#include "dgaproc.h"

#include "xf86Resources.h"
#include "xf86RAC.h"

#include "fb.h"
#define wfbPictureInit fbPictureInit

#include "xaa.h"
#include "vermilion_sys.h"

#define VERMILION_VERSION		4000
#define VERMILION_NAME		"VERMILION"
#define VERMILION_DRIVER_NAME	"vermilion"
#define VERMILION_MAJOR_VERSION	1
#define VERMILION_MINOR_VERSION	0
#define VERMILION_PATCHLEVEL		0

 /*XXX*/ typedef struct _VERMILIONRec
{
    EntityInfoPtr pEnt;
    GDevPtr device;
    pciVideoPtr pciInfo;
    PCITAG pciTag;
    xf86MonPtr monitor;
    CloseScreenProcPtr CloseScreen;
    OptionInfoPtr Options;

    IOADDRESS ioBase;
    pciVideoPtr mbx;
    int fbFD;

/*
 * Map
 */
    void *fbMap;
    CARD32 *vdcRegsBase;
    CARD32 *mbxRegsBase;
    unsigned long fbSize;
    unsigned long mbxSize;
    unsigned long vdcSize;
    unsigned long mchSize;

/* 
 * Mode info
 */

    int x;
    int y;
    int cpp;
    unsigned stride;
    DisplayModeRec curMode;
/*
 *  Panel
 */
    Bool usePanel;
    int panel;

/*
 * System info
 */
    VERMILIONSys *sys;
    PCITAG kernelmbx;
    Bool fusedClock;
    int fusedClockIndex;

/*
 * Acceleration
 */
    Bool accelOn;
    XAAInfoRecPtr accel;
    CARD32 mbxBpp;
    CARD32 mbxFBDevAddr;
    CARD32 *slavePort;
    CARD32 FifoSlots;
    volatile CARD32 *mbxSyncMap;
    CARD32 mbxSyncDevAddr;
    CARD32 ROP;
    CARD32 transEnable;
    CARD32 fillColour;
    CARD32 dir;

/*
 * ShadowFB
 */
    char *shadowmem;
    Bool shadowFB;

/*
 * Debug modesetting
 */   
    Bool debug;

} VERMILIONRec, *VERMILIONPtr;

typedef struct _VERMILIONPanelRec
{
    char *name;
    int clockMax;		       /* kHz */
    int clockMin;		       /* kHz */
    int hActMax;		       /* Clocks */
    int hActMin;		       /* Clocks */
    int hTotMax;		       /* Clocks */
    int hTotMin;		       /* Clocks */
    int vActMax;		       /* Lines */
    int vActMin;		       /* Lines */
    int vTotMax;		       /* Lines */
    int vTotMin;		       /* Lines */
    int hPerMax;		       /* ns */
    int hPerMin;		       /* ns */
    float gamma;                       /* [] */
} VERMILIONPanelRec, *VERMILIONPanelPtr;

#define VERMILIONPTR(_pScrn) ((VERMILIONPtr) (_pScrn)->driverPrivate)
#define ALIGN_TO(_a, _b) (((_a) + (_b) - 1) & ~((_b) - 1))

/*
 * vermilion_accel.c
 */

extern Bool VERMILIONAccelInit(ScreenPtr pScreen);

/*
 * vermilion_mode.c
 */

extern Bool
VERMILIONDoSetMode(ScrnInfoPtr pScrn, DisplayModePtr pMode,
    Bool plane_enable);
extern ModeStatus
VERMILIONValidMode(int scrnIndex, DisplayModePtr mode, Bool verbose,
    int flags);
extern void VERMILIONSetGraphicsOffset(ScrnInfoPtr pScrn, int x, int y);
extern int VERMILIONBlankScreen(ScrnInfoPtr pScrn, Bool blank);
extern int VERMILIONDimScreen(ScrnInfoPtr pScrn, CARD8 level);
extern void VERMILIONDisablePipe(ScrnInfoPtr pScrn);
void VERMILIONWaitForVblank(ScrnInfoPtr pScrn);

/* 
 * vermilion_panels.c
 */

extern VERMILIONPanelPtr VERMILIONPanels[];
extern int VERMILIONNumPanels;

/*
 * vermilion.c
 */

extern void * VERMILIONMapPciVideo(ScrnInfoPtr pScrn, char *deviceType,
				   PCITAG tag, unsigned long offset, 
				   unsigned long size, char type, int mapType);

#endif /* _VERMILION_H_ */
