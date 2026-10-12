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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>


#include "vermilion_kernel.h"
#include "vermilion.h"
#include "vermilion_reg.h"

/* All drivers initialising the SW cursor need this */
#include "mipointer.h"

/* All drivers implementing backing store need this */
#include "mibstore.h"

/* Colormap handling */
#include "micmap.h"
#include "xf86cmap.h"

/* DPMS */
#define DPMS_SERVER
#include <X11/extensions/dpms.h>
#include "xf86Priv.h"

#define KERNELNAME "Vermilion Range"
#define PROCFB "/proc/fb"
#define DEVFB "/dev/fb"

/* Mandatory functions */
static const OptionInfoRec *VERMILIONAvailableOptions(int chipid, int busid);
static void VERMILIONIdentify(int flags);
static Bool VERMILIONProbe(DriverPtr drv, int flags);
static Bool VERMILIONPreInit(ScrnInfoPtr pScrn, int flags);
static Bool VERMILIONScreenInit(int Index, ScreenPtr pScreen, int argc,
    char **argv);
static Bool VERMILIONEnterVT(int scrnIndex, int flags);
static void VERMILIONLeaveVT(int scrnIndex, int flags);
static Bool VERMILIONCloseScreen(int scrnIndex, ScreenPtr pScreen);
static Bool VERMILIONSaveScreen(ScreenPtr pScreen, int mode);

static Bool VERMILIONSwitchMode(int scrnIndex, DisplayModePtr pMode,
    int flags);
static Bool VERMILIONSetMode(ScrnInfoPtr pScrn, DisplayModePtr pMode);
static void VERMILIONAdjustFrame(int scrnIndex, int x, int y, int flags);
static void VERMILIONFreeScreen(int scrnIndex, int flags);
static void VERMILIONFreeRec(ScrnInfoPtr pScrn);

static void
VERMILIONDisplayPowerManagementSet(ScrnInfoPtr pScrn, int mode, int flags);

/* locally used functions */
static Bool VERMILIONMapMem(ScrnInfoPtr pScrn);
static void VERMILIONUnmapMem(ScrnInfoPtr pScrn);
static void VERMILIONLoadPalette(ScrnInfoPtr pScrn, int numColors,
    int *indices, LOCO * colors, VisualPtr pVisual);
static Bool VERMILIONRestore(ScrnInfoPtr pScrn);
static Bool VERMILIONSave(ScrnInfoPtr pScrn);

#ifndef makedev
#define makedev(x,y)    ((dev_t)(((x) << 8) | (y)))
#endif

/* 
 * This contains the functions needed by the server after loading the
 * driver module.  It must be supplied, and gets added the driver list by
 * the Module Setup funtion in the dynamic case.  In the static case a
 * reference to this is compiled in, and this requires that the name of
 * this DriverRec be an upper-case version of the driver name.
 */

_X_EXPORT DriverRec VERMILION = {
    VERMILION_VERSION,
    VERMILION_DRIVER_NAME,
    VERMILIONIdentify,
    VERMILIONProbe,
    VERMILIONAvailableOptions,
    NULL,
    0
};

#define PCI_CHIP_VERMILION  0x5009
#define PCI_CHIP_VM_MBX     0x5002

enum GenericTypes
{
    CHIP_VERMILION
};

/* Supported chipsets */
static SymTabRec VERMILIONChipsets[] = {
    {CHIP_VERMILION, "vermilion"},
    {-1, NULL}
};

static PciChipsets VERMILIONPCIchipsets[] = {
    {CHIP_VERMILION, PCI_CHIP_VERMILION, NULL},
    {-1, -1, RES_UNDEFINED},
};

typedef enum
{
    OPTION_SHADOWFB,
    OPTION_ACCEL,
    OPTION_FUSEDCLOCK,
    OPTION_PANELTYPE,
    OPTION_DEBUG
} VERMILIONOpts;

static const OptionInfoRec VERMILIONOptions[] = {
    {OPTION_SHADOWFB, "ShadowFB", OPTV_BOOLEAN, {0}, FALSE},
    {OPTION_ACCEL, "Accel", OPTV_BOOLEAN, {0}, FALSE},
    {OPTION_FUSEDCLOCK, "FusedClock", OPTV_INTEGER, {0}, FALSE},
    {OPTION_PANELTYPE, "PanelType", OPTV_INTEGER, {0}, FALSE},
    {OPTION_DEBUG, "Debug", OPTV_BOOLEAN, {0}, FALSE},
    {-1, NULL, OPTV_NONE, {0}, FALSE}
};

/*
 * List of symbols from other modules that this module references.  This
 * list is used to tell the loader that it is OK for symbols here to be
 * unresolved providing that it hasn't been told that they haven't been
 * told that they are essential via a call to xf86LoaderReqSymbols() or
 * xf86LoaderReqSymLists().  The purpose is this is to avoid warnings about
 * unresolved symbols that are not required.
 */

static const char *fbSymbols[] = {
    "fbPictureInit",
    "fbScreenInit",
    NULL
};

static const char *wfbSymbols[] = {
    "wfbPictureInit",
    "wfbScreenInit",
    NULL
};

static const char *shadowSymbols[] = {
    "shadowAdd",
    "shadowAlloc",
    "shadowInit",
    "shadowSetup",
    NULL
};

static const char *xaaSymbols[] = {
    "XAACreateInfoRec",
    "XAADestroyInfoRec",
    "XAAInit",
    "XAAGetCopyROP",
    "XAAGetPatternROP",
    NULL
};

static const char *ddcSymbols[] = {
    "xf86PrintEDID",
    "xf86SetDDCproperties",
    NULL
};

#ifdef XFree86LOADER

/* Module loader interface */
static MODULESETUPPROTO(vermilionSetup);

static XF86ModuleVersionInfo vermilionVersionRec = {
    "vermilion",
    MODULEVENDORSTRING,
    MODINFOSTRING1,
    MODINFOSTRING2,
    XORG_VERSION_CURRENT,
    VERMILION_MAJOR_VERSION, VERMILION_MINOR_VERSION, VERMILION_PATCHLEVEL,
    ABI_CLASS_VIDEODRV,		       /* This is a video driver */
    ABI_VIDEODRV_VERSION,
    MOD_CLASS_VIDEODRV,
    {0, 0, 0, 0}
};

/*
 * This data is accessed by the loader.  The name must be the module name
 * followed by "ModuleData".
 */
_X_EXPORT XF86ModuleData vermilionModuleData =
    { &vermilionVersionRec, vermilionSetup, NULL };

static pointer
vermilionSetup(pointer Module, pointer Options, int *ErrorMajor,
    int *ErrorMinor)
{
    static Bool Initialised = FALSE;

    if (!Initialised) {
	Initialised = TRUE;
	xf86AddDriver(&VERMILION, Module, 0);
	LoaderRefSymLists(fbSymbols, wfbSymbols, ddcSymbols, shadowSymbols, xaaSymbols,
	    NULL);
	return (pointer) TRUE;
    }

    if (ErrorMajor)
	*ErrorMajor = LDR_ONCEONLY;
    return (NULL);
}

#endif

static const OptionInfoRec *
VERMILIONAvailableOptions(int chipid, int busid)
{
    return (VERMILIONOptions);
}

static void
VERMILIONIdentify(int flags)
{
    xf86PrintChipsets(VERMILION_NAME, "driver for VERMILION chipsets",
	VERMILIONChipsets);
}

/*
 * This function is called once, at the start of the first server generation to
 * do a minimal probe for supported hardware.
 */

static Bool
VERMILIONProbe(DriverPtr drv, int flags)
{
    Bool foundScreen = FALSE;
    int numDevSections, numUsed;
    GDevPtr *devSections;
    int *usedChips;
    int i;

    /*
     * Find the config file Device sections that match this
     * driver, and return if there are none.
     */
    if ((numDevSections = xf86MatchDevice(VERMILION_NAME, &devSections)) <= 0)
	return (FALSE);

    /* PCI BUS */
    if (xf86GetPciVideoInfo()) {
	numUsed = xf86MatchPciInstances(VERMILION_NAME, PCI_VENDOR_INTEL,
	    VERMILIONChipsets, VERMILIONPCIchipsets,
	    devSections, numDevSections, drv, &usedChips);
	if (numUsed > 0) {
	    if (flags & PROBE_DETECT)
		foundScreen = TRUE;
	    else {
		for (i = 0; i < numUsed; i++) {
		    ScrnInfoPtr pScrn = NULL;

		    /* Allocate a ScrnInfoRec  */
		    if ((pScrn = xf86ConfigPciEntity(pScrn, 0, usedChips[i],
				VERMILIONPCIchipsets, NULL,
				NULL, NULL, NULL, NULL))) {
			pScrn->driverVersion = VERMILION_VERSION;
			pScrn->driverName = VERMILION_DRIVER_NAME;
			pScrn->name = VERMILION_NAME;
			pScrn->Probe = VERMILIONProbe;
			pScrn->PreInit = VERMILIONPreInit;
			pScrn->ScreenInit = VERMILIONScreenInit;
			pScrn->SwitchMode = VERMILIONSwitchMode;
			pScrn->AdjustFrame = VERMILIONAdjustFrame;
			pScrn->EnterVT = VERMILIONEnterVT;
			pScrn->LeaveVT = VERMILIONLeaveVT;
			pScrn->FreeScreen = VERMILIONFreeScreen;
			pScrn->ValidMode = VERMILIONValidMode;
			foundScreen = TRUE;
		    }
		}
	    }
	    xfree(usedChips);
	}
    }

    xfree(devSections);

    return (foundScreen);
}

static VERMILIONPtr
VERMILIONGetRec(ScrnInfoPtr pScrn)
{
    if (!pScrn->driverPrivate)
	pScrn->driverPrivate = xcalloc(sizeof(VERMILIONRec), 1);

    return ((VERMILIONPtr) pScrn->driverPrivate);
}

static void
VERMILIONFreeRec(ScrnInfoPtr pScrn)
{
    VERMILIONPtr pVermilion = VERMILIONGetRec(pScrn);

    xfree(pVermilion->monitor);
    xfree(pScrn->driverPrivate);
    pScrn->driverPrivate = NULL;
}

void *
VERMILIONMapPciVideo(ScrnInfoPtr pScrn, char *deviceType,
    PCITAG tag,
    unsigned long offset, unsigned long size, char type, int mapType)
{
    void *ptr;

    if (type) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "%s device had incorrect pci "
	    "resource type %d\n", deviceType, (int)type);
	return NULL;
    }

    ptr = xf86MapPciMem(pScrn->scrnIndex, mapType, tag, offset, size);
    if (ptr) {
	xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Mapped %s memory at "
	    "offset 0x%08lx size %lu\n", deviceType, offset, size);
    } else {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "Could not map %s memory at "
	    "offset 0x%08lx size %lu\n", deviceType, offset, size);
    }
    return ptr;

}

static int
VERMILIONKernelOpen(ScrnInfoPtr pScrn)
{
#define PROCBUFSIZE 100

    FILE *proc;
    char buffer[PROCBUFSIZE];
    Bool found = FALSE;
    char *eptr;
    int device;
    int ret = -1;

    proc = fopen(PROCFB, "r");

    if (proc == NULL)
	goto out;

    while (!feof(proc)) {
	fgets(buffer, PROCBUFSIZE, proc);
	if (strstr(buffer, KERNELNAME) == NULL)
	    continue;
	found = TRUE;
	break;
    }

    if (!found)
	goto out;

    device = strtol(buffer, &eptr, 10);
    if (buffer == eptr)
	goto out;

    ret = snprintf(buffer, PROCBUFSIZE, "%s%d", DEVFB, device);
    if (ret < 0 || ret >= PROCBUFSIZE) {
	ret = -1;
	goto out;
    }

    ret = open(buffer, O_RDWR);
    if (ret < 0) {
	/* If we can't get /dev/fbX, then try /dev/fb/X */
    	ret = snprintf(buffer, PROCBUFSIZE, "%s/%d", DEVFB, device);
    	if (ret < 0 || ret >= PROCBUFSIZE) {
	    ret = -1;
	    goto out;
    	}
    	ret = open(buffer, O_RDWR);
    }

  out:
    fclose(proc);
    return ret;
}

void
VERMILIONUnmap(void *virtual)
{
}

static Bool
VERMILIONPreInitAccel(ScrnInfoPtr pScrn)
{
    VERMILIONPtr pVermilion = VERMILIONGetRec(pScrn);
    MessageType from;

    from =
	xf86GetOptValBool(pVermilion->Options, OPTION_ACCEL,
	&pVermilion->accelOn)
	? X_CONFIG : X_DEFAULT;

    if (pVermilion->accelOn) {
	if (!xf86LoadSubModule(pScrn, "xaa"))
	    return FALSE;

	xf86LoaderReqSymLists(xaaSymbols, NULL);
    }

    xf86DrvMsg(pScrn->scrnIndex, from, "Acceleration %sabled\n",
	pVermilion->accelOn ? "en" : "dis");

    return TRUE;
}

static Bool
VERMILIONPreInitShadowFB(ScrnInfoPtr pScrn)
{
    VERMILIONPtr pVermilion = VERMILIONGetRec(pScrn);
    MessageType from;

    from =
	xf86GetOptValBool(pVermilion->Options, OPTION_SHADOWFB,
	&pVermilion->shadowFB)
	? X_CONFIG : X_DEFAULT;

    if (pVermilion->shadowFB) {
	if (!xf86LoadSubModule(pScrn, "shadow"))
	    return FALSE;

	xf86LoaderReqSymLists(shadowSymbols, NULL);
    }

    xf86DrvMsg(pScrn->scrnIndex, from, "Shadow framebuffer %sabled\n",
	pVermilion->shadowFB ? "en" : "dis");

    return TRUE;
}

static int
VERMILIONAllocInstance(ScrnInfoPtr pScrn)
{
    VERMILIONPtr pVermilion = VERMILIONPTR(pScrn);

    int ret;
    vml_init_t arg;
    vml_init_req_t *req = &arg.req;
    vml_init_rep_t *rep = &arg.rep;

    req->major = VML_KI_MAJOR;
    req->minor = VML_KI_MINOR;
    req->pipe = 0;
    req->vdc_tag.bus = pVermilion->pciInfo->bus;
    req->vdc_tag.slot = pVermilion->pciInfo->device;
    req->vdc_tag.function = pVermilion->pciInfo->func;

    ret = ioctl(pVermilion->fbFD, VML_INIT_DEVICE, &arg);

    if (!ret) {
	pVermilion->fbSize = rep->vram_contig_size;
	pScrn->memPhysBase = rep->vram_offset;
	pVermilion->kernelmbx = pciTag(rep->gpu_tag.bus,
	    rep->gpu_tag.slot, rep->gpu_tag.function);
	ErrorF("AllocInstance 0x%08lx %ld\n", pScrn->memPhysBase, pVermilion->fbSize);
    }

    return ret;
}

/*
 * This function is called once for each screen at the start of the first
 * server generation to initialise the screen for all server generations.
 */
static Bool
VERMILIONPreInit(ScrnInfoPtr pScrn, int flags)
{
    VERMILIONPtr pVermilion;
    VERMILIONSys *sys;
    DisplayModePtr pMode;
    Gamma gzeros = { 0.0, 0.0, 0.0 };
    rgb rzeros = { 0, 0, 0 };
    pointer pDDCModule;
    int i;
    int ret;
    int mbxCount;
    MessageType from;
    unsigned ssVendor, ssDevice;
    const char *ssName, *fbmod, **fbsym;

    ClockRangePtr clockRanges;

    if (flags & PROBE_DETECT)
	return (FALSE);

    pVermilion = VERMILIONGetRec(pScrn);
    pVermilion->pEnt = xf86GetEntityInfo(pScrn->entityList[0]);

    if (pVermilion->pEnt->location.type == BUS_PCI) {
	pVermilion->pciInfo =
	    xf86GetPciInfoForEntity(pVermilion->pEnt->index);
	pVermilion->pciTag =
	    pciTag(pVermilion->pciInfo->bus, pVermilion->pciInfo->device,
	    pVermilion->pciInfo->func);
    }

    pVermilion->fbFD = VERMILIONKernelOpen(pScrn);

    if (pVermilion->fbFD < 0) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "Failed to open the %s kernel fb driver. Make sure it is loaded.\n",
		   KERNELNAME);
	return FALSE;
    }

    ret = VERMILIONAllocInstance(pScrn);

    if (ret) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
	    "Failed to allocate a vermilion device: %s. Check kernel output.\n",
	    strerror(errno));
	return FALSE;
    }

    pVermilion->sys = sys = VERMILIONCreateSys(pScrn);
    if (!sys) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
	    "Failed to determine system type.\n");
	return FALSE;
    }
    sys->subSys(sys, &ssName, &ssVendor, &ssDevice);
    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
	"Subsystem Vendor 0x%04x Device 0x%04x.\n", ssVendor, ssDevice);
    xf86DrvMsg(pScrn->scrnIndex, X_PROBED, "This is a %s board.\n", ssName);

    pVermilion->device = xf86GetDevFromEntity(pScrn->entityList[0],
	pScrn->entityInstanceList[0]);

    mbxCount = 0;
    do {
	pVermilion->mbx = xf86FindPciDeviceVendor(PCI_VENDOR_INTEL,
	    PCI_CHIP_VM_MBX, mbxCount, NULL);

	/*
	 * Check that the mbx device is the one the kernel allocated for us.
	 */

	if (pVermilion->mbx &&
	    pciTag(pVermilion->mbx->bus,
		pVermilion->mbx->device,
		pVermilion->mbx->func) == pVermilion->kernelmbx)
	    break;

	mbxCount++;
    } while (pVermilion->mbx);

    if (!pVermilion->mbx) {
	ErrorF("Could not find MBX device\n");
	return FALSE;
    }

    if (!xf86SetDepthBpp(pScrn, 15, 0, 0, Support32bppFb)) {
	return (FALSE);
    }
    if (pScrn->depth != 15 && pScrn->depth != 24) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
	    "Invalid depth %d, only 15 and 24 supported\n", pScrn->depth);
	return (FALSE);
    }
    xf86PrintDepthBpp(pScrn);

    if (pScrn->depth == 15) {
	fbmod = "wfb";
	fbsym = wfbSymbols;
    } else {
	fbmod = "fb";
	fbsym = fbSymbols;
    }

    /* Load (w)fb module */
    if (!xf86LoadSubModule(pScrn, fbmod))
	return (FALSE);

    xf86LoaderReqSymLists(fbsym, NULL);

    pScrn->chipset = "vermilion";
    pScrn->monitor = pScrn->confScreen->monitor;
    pScrn->rgbBits = 8;
    pScrn->videoRam = pVermilion->fbSize / 1024;

    xf86DrvMsg(pScrn->scrnIndex, X_PROBED, "Linear framebuffer at 0x%lX\n",
	(unsigned long)pScrn->memPhysBase);

    if (xf86RegisterResources(pVermilion->pEnt->index, NULL, ResExclusive)) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
	    "xf86RegisterResources() found resource conflicts\n");
	return FALSE;
    }

    /* color weight */
    if (!xf86SetWeight(pScrn, rzeros, rzeros)) {
	return (FALSE);
    }
    /* visual init */
    if (!xf86SetDefaultVisual(pScrn, -1)) {
	return (FALSE);
    }

    /* We can't do this until we have a
     * pScrn->display. */
    xf86CollectOptions(pScrn, NULL);
    if (!(pVermilion->Options = xalloc(sizeof(VERMILIONOptions))))
	return (FALSE);

    memcpy(pVermilion->Options, VERMILIONOptions, sizeof(VERMILIONOptions));
    xf86ProcessOptions(pScrn->scrnIndex, pScrn->options, pVermilion->Options);

    pVermilion->shadowFB = FALSE;
    pVermilion->accelOn = TRUE;

    if (!VERMILIONPreInitShadowFB(pScrn))
	return (FALSE);

    if (!pVermilion->shadowFB && !VERMILIONPreInitAccel(pScrn))
	return (FALSE);

    /*
     * Check panel option.
     */

    pVermilion->panel = sys->panel(sys);
    from = xf86GetOptValInteger(pVermilion->Options, OPTION_PANELTYPE,
	&pVermilion->panel)
	? X_CONFIG : X_DEFAULT;

    if (pVermilion->panel >= VERMILIONNumPanels) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
	    "Unknown panel type %d\n", pVermilion->panel);
	return FALSE;
    }
    pVermilion->usePanel = (pVermilion->panel >= 0);
    if (pVermilion->usePanel) {
	xf86DrvMsg(pScrn->scrnIndex, from,
	    "Using panel type %d, which is a %s.\n",
	    pVermilion->panel, VERMILIONPanels[pVermilion->panel]->name);
    } else {
	xf86DrvMsg(pScrn->scrnIndex, from, "Not using panel\n");
    }

    pScrn->progClock = sys->progClock(sys);

    if (!pScrn->progClock)
	sys->clocks(sys, &pScrn->numClocks, pScrn->clock);

    pVermilion->fusedClockIndex = -1;
    from = xf86GetOptValInteger(pVermilion->Options, OPTION_FUSEDCLOCK,
	&pVermilion->fusedClockIndex)
	? X_CONFIG : X_DEFAULT;

    if (pVermilion->panel >= pScrn->numClocks) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
	    "Unknown fused clock index %d\n", pVermilion->fusedClockIndex);
	return FALSE;
    }
    pVermilion->fusedClock = (pVermilion->fusedClockIndex >= 0);
    if (pVermilion->fusedClock) {
	xf86DrvMsg(pScrn->scrnIndex, from,
	    "Fused dotclock index %d which is %d kHz.\n",
	    pVermilion->fusedClockIndex,
	    pScrn->clock[pVermilion->fusedClockIndex]);
    }

    pVermilion->debug = FALSE;
    from =
	xf86GetOptValBool(pVermilion->Options, OPTION_DEBUG,
	&pVermilion->debug)
	? X_CONFIG : X_DEFAULT;

    xf86DrvMsg(pScrn->scrnIndex, from,
	"Modesetting debugging printout is %sabled.\n",
	pVermilion->debug ? "en" : "dis");

    xf86SetGamma(pScrn, gzeros);

    /* Load ddc module */
    if ((pDDCModule = xf86LoadSubModule(pScrn, "ddc")) == NULL) {
#if 0
	if ((pVermilion->monitor =
		vbeDoEDID(pVermilion->pVbe, pDDCModule)) != NULL) {
	    xf86PrintEDID(pVermilion->monitor);
	}
#endif
	xf86UnloadSubModule(pDDCModule);
    }

    if ((pScrn->monitor->DDC = pVermilion->monitor) != NULL)
	xf86SetDDCproperties(pScrn, pVermilion->monitor);

    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
	"Searching for matching VERMILION mode(s):\n");

    clockRanges = xnfcalloc(sizeof(ClockRange), 1);
    clockRanges->next = NULL;
    sys->clockRanges(sys, &clockRanges->minClock, &clockRanges->maxClock);
    clockRanges->clockIndex = -1;
    clockRanges->interlaceAllowed = FALSE;
    clockRanges->doubleScanAllowed = FALSE;
    pVermilion->cpp = pScrn->bitsPerPixel >> 3;
    pScrn->xInc = 1;

    if (pVermilion->fusedClock) {
	pScrn->clock[0] = pScrn->clock[pVermilion->fusedClockIndex];
	pScrn->numClocks = 1;
    }

    i = xf86ValidateModes(pScrn, pScrn->monitor->Modes, pScrn->display->modes,
	clockRanges, NULL, 0, 2048, 64 << 3, 0, 2048,
	pScrn->display->virtualX,
	pScrn->display->virtualY, pScrn->videoRam * 1024,
	pVermilion->usePanel ? LOOKUP_CLOSEST_CLOCK : LOOKUP_BEST_REFRESH);

    if (i <= 0) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "No valid modes\n");
	return (FALSE);
    }

    pVermilion->stride = pVermilion->cpp * pScrn->displayWidth;
    xf86PruneDriverModes(pScrn);

    pMode = pScrn->modes;
    pScrn->currentMode = pScrn->modes;
    xf86PrintModes(pScrn);

    /* Set display resolution */
    xf86SetDpi(pScrn, 0, 0);

    if (pScrn->modes == NULL) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "No modes\n");
	return (FALSE);
    }

    return (TRUE);
}

void
VERMILIONUpdatePackedDepth15(ScreenPtr pScreen, shadowBufPtr pBuf)
{
    RegionPtr damage = &pBuf->damage;
    PixmapPtr pShadow = pBuf->pPixmap;
    int nbox = REGION_NUM_RECTS(damage);
    BoxPtr pbox = REGION_RECTS(damage);
    FbBits *shaBase, *shaLine, *sha;
    FbStride shaStride;
    int scrBase, scrLine, scr;
    int shaBpp;
    int shaXoff, shaYoff;	       /* XXX assumed to be zero */
    int x, y, w, h, width;
    int i;
    FbBits *winBase = NULL, *win;
    CARD32 winSize;

    fbGetDrawable(&pShadow->drawable, shaBase, shaStride, shaBpp, shaXoff,
	shaYoff);
    while (nbox--) {
	x = pbox->x1 * shaBpp;
	y = pbox->y1;
	w = (pbox->x2 - pbox->x1) * shaBpp;
	h = pbox->y2 - pbox->y1;

	scrLine = (x >> FB_SHIFT);
	shaLine = shaBase + y * shaStride + (x >> FB_SHIFT);

	x &= FB_MASK;
	w = (w + x + FB_MASK) >> FB_SHIFT;

	while (h--) {
	    winSize = 0;
	    scrBase = 0;
	    width = w;
	    scr = scrLine;
	    sha = shaLine;
	    while (width) {
		/* how much remains in this window */
		i = scrBase + winSize - scr;
		if (i <= 0 || scr < scrBase) {
		    winBase = (FbBits *) (*pBuf->window) (pScreen,
			y,
			scr * sizeof(FbBits),
			SHADOW_WINDOW_WRITE, &winSize, pBuf->closure);
		    if (!winBase)
			return;
		    scrBase = scr;
		    winSize /= sizeof(FbBits);
		    i = winSize;
		}
		win = winBase + (scr - scrBase);
		if (i > width)
		    i = width;
		width -= i;
		scr += i;
		/* Here we set the high bit on upload for depth 15 because
		 * the hardware requires it. - AlanH.
		 */
		while (i--)
		    *win++ = 0x80008000 | *sha++;
	    }
	    shaLine += shaStride;
	    y++;
	}
	pbox++;
    }
}

void
VERMILIONUpdatePackedDepth24(ScreenPtr pScreen, shadowBufPtr pBuf)
{
    RegionPtr damage = &pBuf->damage;
    PixmapPtr pShadow = pBuf->pPixmap;
    int nbox = REGION_NUM_RECTS(damage);
    BoxPtr pbox = REGION_RECTS(damage);
    FbBits *shaBase, *shaLine, *sha;
    FbStride shaStride;
    int scrBase, scrLine, scr;
    int shaBpp;
    int shaXoff, shaYoff;	       /* XXX assumed to be zero */
    int x, y, w, h, width;
    int i;
    FbBits *winBase = NULL, *win;
    CARD32 winSize;

    fbGetDrawable(&pShadow->drawable, shaBase, shaStride, shaBpp, shaXoff,
	shaYoff);
    while (nbox--) {
	x = pbox->x1 * shaBpp;
	y = pbox->y1;
	w = (pbox->x2 - pbox->x1) * shaBpp;
	h = pbox->y2 - pbox->y1;

	scrLine = (x >> FB_SHIFT);
	shaLine = shaBase + y * shaStride + (x >> FB_SHIFT);

	x &= FB_MASK;
	w = (w + x + FB_MASK) >> FB_SHIFT;

	while (h--) {
	    winSize = 0;
	    scrBase = 0;
	    width = w;
	    scr = scrLine;
	    sha = shaLine;
	    while (width) {
		/* how much remains in this window */
		i = scrBase + winSize - scr;
		if (i <= 0 || scr < scrBase) {
		    winBase = (FbBits *) (*pBuf->window) (pScreen,
			y,
			scr * sizeof(FbBits),
			SHADOW_WINDOW_WRITE, &winSize, pBuf->closure);
		    if (!winBase)
			return;
		    scrBase = scr;
		    winSize /= sizeof(FbBits);
		    i = winSize;
		}
		win = winBase + (scr - scrBase);
		if (i > width)
		    i = width;
		width -= i;
		scr += i;
		while (i--)
		    *win++ = *sha++;
	    }
	    shaLine += shaStride;
	    y++;
	}
	pbox++;
    }
}

static void *
VERMILIONWindowLinear(ScreenPtr pScreen, CARD32 row, CARD32 offset, int mode,
    CARD32 * size, void *closure)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    VERMILIONPtr pVermilion = VERMILIONGetRec(pScrn);

    if (!pScrn->vtSema)
	return NULL;

    *size = pVermilion->stride;

    return ((CARD8 *) pVermilion->fbMap + row * (*size) + offset);
}

#if X_BYTE_ORDER == X_BIG_ENDIAN
#error VERMILIONReadMemory and VERMILIONWriteMemory only work on little endian
#endif

static FbBits
VERMILIONReadMemory(const void *src, int size)
{
    FbBits bits = 0;

    memcpy(&bits, src, size);

    return bits;
}

static void
VERMILIONWriteMemoryPassthru(void *dst, FbBits value, int size)
{
    memcpy(dst, &value, size);
}

static void
VERMILIONWriteMemorySetAlpha(void *dst, FbBits value, int size)
{
    switch (size) {
    case 4:
	value |= 0x80008000;
	break;
    case 3:
    case 2:
	value |= 0x8000;
    case 1:
	break;
    default:
	FatalError("Unsupported size %d in %s", size, __func__);
    }

    memcpy(dst, &value, size);
}

static void
VERMILIONSetupWrap(ReadMemoryProcPtr *pRead, WriteMemoryProcPtr *pWrite,
			DrawablePtr pDraw)
{
    ScreenPtr pScreen = pDraw->pScreen;
    ScrnInfoPtr pScrn = xf86Screens[pDraw->pScreen->myNum];
    VERMILIONPtr pVermilion = VERMILIONGetRec(pScrn);
    PixmapPtr pPixmap;

    if (pDraw->type == DRAWABLE_WINDOW)
	pPixmap = pScreen->GetWindowPixmap((WindowPtr)pDraw);
    else
	pPixmap = (PixmapPtr)pDraw;

    if (pScrn->depth == 15 && pPixmap->devPrivate.ptr >= pVermilion->fbMap &&
	(char*)pPixmap->devPrivate.ptr < ((char*)pVermilion->fbMap +
					   pVermilion->fbSize)) {
	*pWrite = VERMILIONWriteMemorySetAlpha;
    } else {
	*pWrite = VERMILIONWriteMemoryPassthru;
    }

    *pRead = VERMILIONReadMemory;
}

static void
VERMILIONFinishWrap(DrawablePtr pDraw) 
{
}

static Bool
VERMILIONScreenInit(int scrnIndex, ScreenPtr pScreen, int argc, char **argv)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
    VERMILIONPtr pVermilion = VERMILIONGetRec(pScrn);
    VisualPtr visual;
    int flags;
    char *fbstart;
    VERMILIONSys *sys = pVermilion->sys;

    if (!VERMILIONMapMem(pScrn)) {
	return (FALSE);
    }

    /* save current video state */
    VERMILIONSave(pScrn);

    /* set the viewport */
    VERMILIONAdjustFrame(scrnIndex, pScrn->frameX0, pScrn->frameY0, 0);

    /* Clear framebuffer contents before setting initial mode to prevent garbage
     * or even screen contents from a previous session from being visible.
     */

    if (pScrn->bitsPerPixel == 32) {
	memset(pVermilion->fbMap, 0, pScrn->virtualY * pVermilion->stride);
    } else {
	/* For 16 bpp, make sure the alpha bit is set. Gets trashed later on
	 * unfortunately.
	 */
	int y = pScrn->virtualY;

	while (y--) {
	    int x;
	    CARD32 *pixels = (CARD32 *) ((char *)pVermilion->fbMap +
		y * pVermilion->stride);

	    for (x = pScrn->virtualX; x; x -= 2, pixels++)
		*pixels = 0x80008000;
	}
    }

    if (pVermilion->usePanel) {
	sys->panelOn(sys);
    }

    /* set first video mode */
    if (!VERMILIONSetMode(pScrn, pScrn->currentMode))
	return (FALSE);

    /* Blank the screen for aesthetic reasons. */
    VERMILIONSaveScreen(pScreen, SCREEN_SAVER_ON);

    /* mi layer */
    miClearVisualTypes();
    if (!xf86SetDefaultVisual(pScrn, -1))
	return (FALSE);
    if (!miSetVisualTypes(pScrn->depth, TrueColorMask,
	    pScrn->rgbBits, TrueColor))
	return (FALSE);
    if (!miSetPixmapDepths())
	return (FALSE);

    /* shadowfb */
    if (pVermilion->shadowFB) {
	if ((pVermilion->shadowmem =
		shadowAlloc(pScrn->displayWidth, pScrn->virtualY,
		    pScrn->bitsPerPixel)) == NULL) {
	    xf86DrvMsg(scrnIndex, X_ERROR,
		"Allocation of shadow memory failed\n");
	    return FALSE;
	}
	fbstart = pVermilion->shadowmem;
    } else {
	fbstart = pVermilion->fbMap;
    }

    if (!(pScrn->depth == 15 ?
	  wfbScreenInit(pScreen, fbstart, pScrn->virtualX, pScrn->virtualY,
			pScrn->xDpi, pScrn->yDpi, pScrn->displayWidth,
			pScrn->bitsPerPixel, VERMILIONSetupWrap,
			VERMILIONFinishWrap) :
	  fbScreenInit(pScreen, fbstart, pScrn->virtualX, pScrn->virtualY,
		       pScrn->xDpi, pScrn->yDpi, pScrn->displayWidth,
		       pScrn->bitsPerPixel)))
	return (FALSE);

    /* Fixup RGB ordering */
    visual = pScreen->visuals + pScreen->numVisuals;
    while (--visual >= pScreen->visuals) {
	if ((visual->class | DynamicClass) == DirectColor) {
	    visual->offsetRed = pScrn->offset.red;
	    visual->offsetGreen = pScrn->offset.green;
	    visual->offsetBlue = pScrn->offset.blue;
	    visual->redMask = pScrn->mask.red;
	    visual->greenMask = pScrn->mask.green;
	    visual->blueMask = pScrn->mask.blue;
	}
    }

    /* must be after RGB ordering fixed */
    if (pScrn->depth == 15)
        wfbPictureInit(pScreen, 0, 0);
    else
    	fbPictureInit(pScreen, 0, 0);

    if (pVermilion->shadowFB &&
	(!shadowSetup(pScreen) || !shadowAdd(pScreen, NULL,
		pScrn->depth ==
		15 ? VERMILIONUpdatePackedDepth15 :
		VERMILIONUpdatePackedDepth24, VERMILIONWindowLinear, 0,
		NULL))) {
	xf86DrvMsg(scrnIndex, X_ERROR,
	    "Shadow framebuffer initialization failed.\n");
	return FALSE;
    }

    xf86SetBlackWhitePixels(pScreen);
    miInitializeBackingStore(pScreen);
    xf86SetBackingStore(pScreen);

    /* Enable acceleration */
    if (!pVermilion->shadowFB && pVermilion->accelOn) {
	if (!VERMILIONAccelInit(pScreen)) {
	    xf86DrvMsg(scrnIndex, X_ERROR,
		"Acceleration initialization failed\n");
	    pVermilion->accelOn = FALSE;
	}
    }

    /* software cursor */
    miDCInitialize(pScreen, xf86GetPointerScreenFuncs());

    /* colormap */
    if (!miCreateDefColormap(pScreen))
	return (FALSE);

    flags = CMAP_RELOAD_ON_MODE_SWITCH;

    if (!xf86HandleColormaps(pScreen, 256, 8,
	    VERMILIONLoadPalette, NULL, flags))
	return (FALSE);

    pVermilion->CloseScreen = pScreen->CloseScreen;
    pScreen->CloseScreen = VERMILIONCloseScreen;
    pScreen->SaveScreen = VERMILIONSaveScreen;

    xf86DPMSInit(pScreen, VERMILIONDisplayPowerManagementSet, 0);

    /* Report any unused options (only for the first generation) */
    if (serverGeneration == 1)
	xf86ShowUnusedOptions(pScrn->scrnIndex, pScrn->options);

    return (TRUE);
}

static Bool
VERMILIONEnterVT(int scrnIndex, int flags)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
    VERMILIONPtr pVermilion = VERMILIONPTR(pScrn);
    VERMILIONSys *sys = pVermilion->sys;

    VERMILIONAdjustFrame(scrnIndex, pScrn->frameX0, pScrn->frameY0, 0);
    if (!VERMILIONSetMode(pScrn, pScrn->currentMode))
	return FALSE;

    /* Power up the panel */
    if (pVermilion->usePanel) {
	sys->panelOn(sys);
    }

    return TRUE;
}

static void
VERMILIONLeaveVT(int scrnIndex, int flags)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
    VERMILIONPtr pVermilion = VERMILIONPTR(pScrn);

    if (pVermilion->accel)
	(*pVermilion->accel->Sync) (pScrn);

    /* clear the framebuffer when we switch */
    memset(pVermilion->fbMap, 0, pScrn->virtualY * pVermilion->stride);

    VERMILIONDisablePipe(pScrn);
    VERMILIONRestore(pScrn);

    pScrn->vtSema = FALSE;
}

static Bool
VERMILIONCloseScreen(int scrnIndex, ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
    VERMILIONPtr pVermilion = VERMILIONPTR(pScrn);

    if (pVermilion->accel) {
	(*pVermilion->accel->Sync) (pScrn);
	XAADestroyInfoRec(pVermilion->accel);
	pVermilion->accel = NULL;
    }

    if (pScrn->vtSema) {
	VERMILIONDisablePipe(pScrn);
	VERMILIONRestore(pScrn);
	VERMILIONUnmapMem(pScrn);
    }
    pScrn->vtSema = FALSE;

    pScreen->CloseScreen = pVermilion->CloseScreen;
    return pScreen->CloseScreen(scrnIndex, pScreen);
}

static Bool
VERMILIONSwitchMode(int scrnIndex, DisplayModePtr pMode, int flags)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
    VERMILIONPtr pVermilion = VERMILIONPTR(pScrn);

    if (pVermilion->accel)
	(*pVermilion->accel->Sync) (pScrn);

    return VERMILIONSetMode(xf86Screens[scrnIndex], pMode);
}

/* Set a graphics mode */
static Bool
VERMILIONSetMode(ScrnInfoPtr pScrn, DisplayModePtr pMode)
{
    pScrn->vtSema = VERMILIONDoSetMode(pScrn, pMode, 1);
    return pScrn->vtSema;
}

static void
VERMILIONAdjustFrame(int scrnIndex, int x, int y, int flags)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
    VERMILIONPtr pVermilion = VERMILIONPTR(pScrn);

    VERMILIONSetGraphicsOffset(pScrn, x, y);
    pVermilion->x = x;
    pVermilion->y = y;
}

static void
VERMILIONFreeScreen(int scrnIndex, int flags)
{
    VERMILIONFreeRec(xf86Screens[scrnIndex]);
}

static Bool
VERMILIONMapMem(ScrnInfoPtr pScrn)
{
    VERMILIONPtr pVermilion = VERMILIONPTR(pScrn);

    if (pVermilion->fbMap != NULL)
	return (TRUE);

    pVermilion->mbxRegsBase = NULL;
    pVermilion->vdcRegsBase = NULL;

    pVermilion->fbMap = mmap(NULL, pVermilion->fbSize, PROT_READ | PROT_WRITE,
	MAP_SHARED, pVermilion->fbFD, 0);

    if (!pVermilion->fbMap || pVermilion->fbMap == MAP_FAILED) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "Failed to map video memory\n");
	return FALSE;
    }

    pVermilion->mbxSize = 1 << pVermilion->mbx->size[0];
    pVermilion->mbxRegsBase = VERMILIONMapPciVideo(pScrn, "MBX",
	pciTag(pVermilion->mbx->bus,
	    pVermilion->mbx->device,
	    pVermilion->mbx->func),
	pVermilion->mbx->memBase[0],
	pVermilion->mbxSize, pVermilion->mbx->type[0], VIDMEM_MMIO);

    if (!pVermilion->mbxRegsBase) {
	VERMILIONUnmapMem(pScrn);
	return FALSE;
    }

    pVermilion->vdcSize = 1 << pVermilion->pciInfo->size[0];
    pVermilion->vdcRegsBase = VERMILIONMapPciVideo(pScrn, "Video Controller",
	pVermilion->pciTag,
	pVermilion->pciInfo->memBase[0],
	pVermilion->vdcSize, pVermilion->pciInfo->type[0], VIDMEM_MMIO);

    if (!pVermilion->vdcRegsBase) {
	VERMILIONUnmapMem(pScrn);
	return FALSE;
    }

    return TRUE;
}

static void
VERMILIONUnmapMem(ScrnInfoPtr pScrn)
{
    VERMILIONPtr pVermilion = VERMILIONPTR(pScrn);

    if (pVermilion->fbMap != NULL) {
	munmap(pVermilion->fbMap, pVermilion->fbSize);
	pVermilion->fbMap = NULL;
    }

    if (pVermilion->mbxRegsBase != NULL) {
	xf86UnMapVidMem(pScrn->scrnIndex, pVermilion->mbxRegsBase,
	    pVermilion->mbxSize);
	pVermilion->mbxRegsBase = NULL;
    }

    if (pVermilion->vdcRegsBase != NULL) {
	xf86UnMapVidMem(pScrn->scrnIndex, pVermilion->vdcRegsBase,
	    pVermilion->vdcSize);
	pVermilion->vdcRegsBase = NULL;
    }
}

static void
VERMILIONLoadPalette(ScrnInfoPtr pScrn, int numColors, int *indices,
    LOCO * colors, VisualPtr pVisual)
{
    /* 
     * Do we ever use this function? 
     */
}

static Bool
VERMILIONSaveScreen(ScreenPtr pScreen, int mode)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    VERMILIONPtr pVermilion = VERMILIONPTR(pScrn);
    Bool on = xf86IsUnblank(mode);
    VERMILIONSys *sys = pVermilion->sys;

    if (on)
	SetTimeSinceLastInputEvent();

    if (pScrn->vtSema) {
	VERMILIONBlankScreen(pScrn, !on);
	/* Power up the panel */
	if (pVermilion->usePanel) {
	    if (on) {
		sys->backlightOn(sys);
	    } else {
		sys->backlightOff(sys);
	    }
	}
    }

    return (TRUE);
}

Bool
VERMILIONSave(ScrnInfoPtr pScrn)
{
    VERMILIONPtr pVermilion = VERMILIONPTR(pScrn);
    VERMILIONSys *sys = pVermilion->sys;

    sys->save(sys);
    return (TRUE);
}

Bool
VERMILIONRestore(ScrnInfoPtr pScrn)
{
    VERMILIONPtr pVermilion = VERMILIONPTR(pScrn);
    VERMILIONSys *sys = pVermilion->sys;

    sys->restore(sys);
    return (TRUE);
}

static void
VERMILIONDisplayPowerManagementSet(ScrnInfoPtr pScrn, int mode, int flags)
{
    VERMILIONPtr pVermilion = VERMILIONPTR(pScrn);
    VERMILIONSys *sys = pVermilion->sys;

    if (!pScrn->vtSema)
	return;

    switch (mode) {
    case DPMSModeOn:
	if (pVermilion->usePanel) {
	    sys->panelOn(sys);
	    sys->backlightOn(sys);
	}
	VERMILIONDoSetMode(pScrn, &pVermilion->curMode, 1);
	/* Screen: On; HSync: On, VSync: On */
	break;
    case DPMSModeStandby:
	if (pVermilion->usePanel) {
	    sys->backlightOff(sys);
	}
	/* Screen: Off; HSync: Off, VSync: On -- Not Supported */
	break;
    case DPMSModeSuspend:
	if (pVermilion->usePanel) {
	    sys->backlightOff(sys);
	    sys->panelOff(sys);
	}
	/* Screen: Off; HSync: On, VSync: Off -- Not Supported */
	break;
    case DPMSModeOff:
	if (pVermilion->usePanel) {
	    sys->backlightOff(sys);
	    sys->panelOff(sys);
	}
	VERMILIONDisablePipe(pScrn);
	/* Screen: Off; HSync: Off, VSync: Off */
	break;
    }
}
