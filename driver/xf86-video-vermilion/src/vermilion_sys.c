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
 *   Thomas Hellström <thomas-at-tungstengraphics-dot-com>
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "vermilion_kernel.h"
#include "vermilion.h"

typedef enum
{
    carilloRanch = 0,
    numSystems
} VERMILIONSysID;

typedef struct
{
    VERMILIONSysID sysID;
    CARD32 subsys0;
    CARD32 subsys1;
    const char *sysName;
} VERMILIONSubSystem;

/*
 * We can possibly identify the board type 
 * based on the PCI subsystem ids
 * of the host bridge?
 * See the sys initialization function below.
 */

static VERMILIONSubSystem vSystems[] = {
    {carilloRanch, 0x8086, 0x5001, "Carillo Ranch"}
};

/*
*****************************************************************************
* Carillo Ranch System info.
*/

/* The LVDS- and panel power controls sits on the 
   GPIO port of the ISA bridge. */

#define VML_CR_DEVICE_LPC    0x27B8
#define VML_CR_REG_GPIOBAR   0x48
#define VML_CR_REG_GPIOEN    0x4C
#define VML_CR_GPIOEN_BIT    (1 << 4)
#define VML_CR_PANEL_PORT    0x38
#define VML_CR_LVDS_ON       0x00000001
#define VML_CR_PANEL_ON      0x00000002
#define VML_CR_BACKLIGHT_OFF 0x00000004

/* The PLL Clock register sits on Host bridge */
#define VML_CR_DEVICE_MCH   0x5001
#define VML_CR_REG_MCHBAR   0x44
#define VML_CR_REG_MCHEN    0x54
#define VML_CR_MCHEN_BIT    (1 << 28)
#define VML_CR_MCHMAP_SIZE  4096
#define VML_CR_REG_CLOCK    0xc3c
#define VML_CR_CLOCK_SHIFT  8
#define VML_CR_CLOCK_MASK   0x00000f00

typedef struct _CRSys
{
    CARD32 mchBAR;
    unsigned char *mchRegsBase;
    CARD32 gpioBAR;
    CARD32 savedPanelState;
    CARD32 savedClock;
    ScrnInfoPtr pScrn;
} CRSys;

static const unsigned vermilionCRClocks[] = {
    6750,
    13500,
    27000,
    29700,
    37125,
    54000,
    59400,
    74250,
    120000
	/*
	 * There are more clocks, but they are disabled on the CR board.
	 */
};

static const unsigned vermilionCRClockBits[] = {
    0x0a,
    0x09,
    0x08,
    0x07,
    0x06,
    0x05,
    0x04,
    0x03,
    0x0b
};

static const unsigned vermilionCRNumClocks =
    sizeof(vermilionCRClocks) / sizeof(unsigned);

static pointer
VERMILIONCRInit(ScrnInfoPtr pScrn)
{
    CRSys *crSys;
    CARD32 devEn;
    PCITAG curTag;
    PCITAG mchTag = pciTag(0x00, 0x00, 0x00);
    PCITAG lpcTag = pciTag(0x00, 0x1f, 0x00);

    crSys = (CRSys *) calloc(sizeof(*crSys), 1);
    if (!crSys) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "Out of memory.\n");
	return NULL;
    }
    crSys->pScrn = pScrn;

    curTag = pciFindFirst((VML_CR_DEVICE_MCH << 16) | PCI_VENDOR_INTEL,
	0xffffffff);
    if (curTag != mchTag) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "Could not find Carillo Ranch "
	    "MCH device.\n");
	goto out_err;
    }

    devEn = (CARD32) pciReadLong(mchTag, VML_CR_REG_MCHEN);
    if (!(devEn & VML_CR_MCHEN_BIT)) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "Carillo Ranch MCH device "
	    "was not enabled.\n");
	goto out_err;
    }

    crSys->mchBAR = (CARD32) pciReadLong(mchTag, VML_CR_REG_MCHBAR);
    crSys->mchRegsBase =
	VERMILIONMapPciVideo(pScrn, "MCH",
	mchTag, crSys->mchBAR, VML_CR_MCHMAP_SIZE, 0, VIDMEM_MMIO);
    if (!crSys->mchRegsBase)
	goto out_err;

    /*
     * Get the gpio bar.
     */

    curTag = pciFindFirst((VML_CR_DEVICE_LPC << 16) | PCI_VENDOR_INTEL,
	0xffffffff);
    if (curTag != lpcTag) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "Could not find Carillo Ranch "
	    "LPC device.\n");
	goto out_err;
    }

    devEn = pciReadByte(lpcTag, VML_CR_REG_GPIOEN);
    if (!(devEn & VML_CR_GPIOEN_BIT)) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "Carillo Ranch GPIO "
	    "was not enabled.\n");
	goto out_err;
    }

    crSys->gpioBAR = (CARD32) pciReadLong(lpcTag, VML_CR_REG_GPIOBAR) & ~0x3F;
    return crSys;

  out_err:

    if (crSys->mchRegsBase)
	xf86UnMapVidMem(crSys->pScrn->scrnIndex, crSys->mchRegsBase,
	    VML_CR_MCHMAP_SIZE);

    free(crSys);
    return NULL;
}

static void
VERMILIONCRSysDestroy(VERMILIONSys * sys)
{
    CRSys *crSys = (CRSys *) sys->priv;

    if (crSys->mchRegsBase)
	xf86UnMapVidMem(crSys->pScrn->scrnIndex, crSys->mchRegsBase,
	    VML_CR_MCHMAP_SIZE);

    free(crSys);
    free(sys);
}

static void
VERMILIONCRPanelOn(const VERMILIONSys * sys)
{
    CRSys *crSys = (CRSys *) sys->priv;
    CARD32 addr = crSys->gpioBAR + VML_CR_PANEL_PORT;
    CARD32 cur = inl(addr);

    if (!(cur & VML_CR_PANEL_ON)) {
	/* Make sure LVDS controller is down. */
	if (cur & 0x00000001) {
	    cur &= ~VML_CR_LVDS_ON;
	    outl(addr, cur);
	}
	/* Power up Panel */
	usleep(100000);
	cur |= VML_CR_PANEL_ON;
	outl(addr, cur);
    }

    /* Power up LVDS controller */

    if (!(cur & VML_CR_LVDS_ON)) {
	usleep(100000);
	outl(addr, cur | VML_CR_LVDS_ON);
    }
}

void
VERMILIONCRPanelOff(const VERMILIONSys * sys)
{
    CRSys *crSys = (CRSys *) sys->priv;
    CARD32 addr = crSys->gpioBAR + VML_CR_PANEL_PORT;
    CARD32 cur = inl(addr);

    /* Power down LVDS controller first to avoid high currents */
    if (cur & VML_CR_LVDS_ON) {
	cur &= ~VML_CR_LVDS_ON;
	outl(addr, cur);
    }
    if (cur & VML_CR_PANEL_ON) {
	usleep(100000);
	outl(addr, cur & ~VML_CR_PANEL_ON);
    }
}

void
VERMILIONCRBacklightOn(const VERMILIONSys * sys)
{
    CRSys *crSys = (CRSys *) sys->priv;
    CARD32 addr = crSys->gpioBAR + VML_CR_PANEL_PORT;
    CARD32 cur = inl(addr);

    if (cur & VML_CR_BACKLIGHT_OFF) {
	cur &= ~VML_CR_BACKLIGHT_OFF;
	outl(addr, cur);
    }
}

void
VERMILIONCRBacklightOff(const VERMILIONSys * sys)
{
    CRSys *crSys = (CRSys *) sys->priv;
    CARD32 addr = crSys->gpioBAR + VML_CR_PANEL_PORT;
    CARD32 cur = inl(addr);

    if (!(cur & VML_CR_BACKLIGHT_OFF)) {
	cur |= VML_CR_BACKLIGHT_OFF;
	outl(addr, cur);
    }
}

static Bool
VERMILIONCRSysRestore(VERMILIONSys * sys)
{
    CRSys *crSys = (CRSys *) sys->priv;
    volatile CARD32 *clockReg = (volatile CARD32 *)
	(crSys->mchRegsBase + VML_CR_REG_CLOCK);

    CARD32 cur = crSys->savedPanelState;

    if (cur & VML_CR_BACKLIGHT_OFF) {
	VERMILIONCRBacklightOff(sys);
    } else {
	VERMILIONCRBacklightOn(sys);
    }

    if (cur & VML_CR_PANEL_ON) {
	VERMILIONCRPanelOn(sys);
    } else {
	VERMILIONCRPanelOff(sys);
	if (cur & VML_CR_LVDS_ON) {
	    ;
	    /* Will not power up LVDS controller while panel is off */
	}
    }

    *clockReg = crSys->savedClock;
    (void)*clockReg;

    return TRUE;
}

static Bool
VERMILIONCRSysSave(VERMILIONSys * sys)
{
    CRSys *crSys = (CRSys *) sys->priv;
    volatile CARD32 *clockReg = (volatile CARD32 *)
	(crSys->mchRegsBase + VML_CR_REG_CLOCK);

    crSys->savedPanelState = inl(crSys->gpioBAR + VML_CR_PANEL_PORT);
    crSys->savedClock = *clockReg;

    return TRUE;
}

static Bool
VERMILIONCRSetClock(VERMILIONSys * sys, int clock)
{
    CRSys *crSys = (CRSys *) sys->priv;
    volatile CARD32 *clockReg = (volatile CARD32 *)
	(crSys->mchRegsBase + VML_CR_REG_CLOCK);
    int index;
    CARD32 clockVal;

    index = xf86GetNearestClock(crSys->pScrn, clock, FALSE, 1, 1, NULL);
    if (vermilionCRClocks[index] != clock)
	return FALSE;

    clockVal = *clockReg & ~VML_CR_CLOCK_MASK;
    clockVal = vermilionCRClockBits[index] << VML_CR_CLOCK_SHIFT;
    *clockReg = clockVal;
    (void)*clockReg;

    return TRUE;
}

static void
VERMILIONCRClocks(const VERMILIONSys * sys, int *numClocks, int clocks[])
{
    unsigned i;

    *numClocks = vermilionCRNumClocks;
    for (i = 0; i < vermilionCRNumClocks; ++i) {
	clocks[i] = vermilionCRClocks[i];
    }
}

/*
*********************Generic functions******************************
*/

static void
VERMILIONGenericSubSys(VERMILIONSys * sys, char const **name,
    unsigned *id0, unsigned *id1)
{
    VERMILIONSubSystem *subSys = &vSystems[(VERMILIONSysID) sys->id];

    *name = subSys->sysName;
    *id0 = subSys->subsys0;
    *id1 = subSys->subsys1;
}

static Bool
VERMILIONFalse(VERMILIONSys * sys)
{
    return FALSE;
}

static int
VERMILIONPanel0(const VERMILIONSys * sys)
{
    return 0;
}

static void
VERMILIONGenericClockRanges(const VERMILIONSys * sys, int *low, int *high)
{
    *low = 6500;
    *high = 120000;
}

VERMILIONSys *
VERMILIONCreateSys(ScrnInfoPtr pScrn)
{
    VERMILIONSys *sys;
    VERMILIONSysID sysID;

    sys = (VERMILIONSys *) malloc(sizeof(*sys));
    if (!sys)
	goto out_error;

    /*
     * Add a smart way to detect board id here.
     */

    sysID = carilloRanch;

    sys->id = (int)sysID;
    switch (sysID) {
    case carilloRanch:
	sys->priv = VERMILIONCRInit(pScrn);
	if (!sys->priv)
	    goto out_error;

	sys->subSys = VERMILIONGenericSubSys;
	sys->save = VERMILIONCRSysSave;
	sys->restore = VERMILIONCRSysRestore;
	sys->destroy = VERMILIONCRSysDestroy;
	sys->progClock = VERMILIONFalse;
	sys->clockRanges = VERMILIONGenericClockRanges;
	sys->clocks = VERMILIONCRClocks;
	sys->setClock = VERMILIONCRSetClock;
	sys->panel = VERMILIONPanel0;
	sys->panelOn = VERMILIONCRPanelOn;
	sys->panelOff = VERMILIONCRPanelOff;
	sys->backlightOn = VERMILIONCRBacklightOn;
	sys->backlightOff = VERMILIONCRBacklightOff;
	break;
    default:
	goto out_error;
    }

    return sys;

  out_error:
    if (sys)
	free(sys);
    return NULL;
}
