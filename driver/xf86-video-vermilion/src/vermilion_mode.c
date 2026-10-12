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
 *   Part of this code is taken from the xf86-video-intel driver.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "vermilion.h"
#include "vermilion_reg.h"
#include <math.h>

void
VERMILIONSetGraphicsOffset(ScrnInfoPtr pScrn, int x, int y)
{
    VERMILIONPtr pVermilion = VERMILIONPTR(pScrn);

    VML_WRITE32(VML_DSPCADDR, (CARD32) pScrn->memPhysBase +
	y * pVermilion->stride + x * pVermilion->cpp);
    (void)VML_READ32(VML_DSPCADDR);
}

static int
VERMILIONNearestClock(ScrnInfoPtr pScrn, int clock, int *index)
{
    *index = xf86GetNearestClock(pScrn, clock, FALSE, 1, 1, NULL);

    return pScrn->clock[*index];
}

ModeStatus
VERMILIONValidMode(int scrnIndex, DisplayModePtr mode, Bool verbose,
    int flags)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
    VERMILIONPtr pVermilion = VERMILIONPTR(pScrn);
    int realClock;
    int hPeriod;
    int dummy;

    xf86DrvMsg(scrnIndex, X_INFO, "VERMILIONValidMode: Validating %s (%d)\n",
	mode->name, mode->Clock);

    realClock = VERMILIONNearestClock(pScrn, mode->Clock, &dummy);

    if (mode->Flags & V_INTERLACE)
	return MODE_NO_INTERLACE;

    if (pVermilion->usePanel) {
	VERMILIONPanelPtr panel = VERMILIONPanels[pVermilion->panel];

	if (realClock < panel->clockMin)
	    return MODE_CLOCK_LOW;
	if (realClock > panel->clockMax)
	    return MODE_CLOCK_HIGH;
	if (mode->CrtcHTotal < panel->hTotMin ||
	    mode->CrtcHTotal > panel->hTotMax)
	    return MODE_BAD_HVALUE;
	if (mode->CrtcHDisplay < panel->hActMin ||
	    mode->CrtcHDisplay > panel->hActMax)
	    return MODE_BAD_HVALUE;
	if (mode->CrtcVTotal < panel->vTotMin ||
	    mode->CrtcVTotal > panel->vTotMax)
	    return MODE_BAD_VVALUE;
	if (mode->CrtcVDisplay < panel->vActMin ||
	    mode->CrtcVDisplay > panel->vActMax)
	    return MODE_BAD_VVALUE;

	hPeriod = mode->CrtcHTotal * 10000 / (realClock / 100);
	if (hPeriod < panel->hPerMin || hPeriod > panel->hPerMax) {
	    return MODE_H_ILLEGAL;
	}
    }
    return MODE_OK;
}

void
VERMILIONWaitForVblank(ScrnInfoPtr pScrn)
{
    /* Wait for 20ms, i.e. one cycle at 50hz. */
    usleep(20000);
}

int
VERMILIONBlankScreen(ScrnInfoPtr pScrn, Bool blank)
{
    VERMILIONPtr pVermilion = VERMILIONPTR(pScrn);
    CARD32 cur = VML_READ32(VML_PIPEACONF);

    /*
     * We'd like to force planes off here, but then we can't
     * force them on again. Hw bug?
     */

    if (blank) {
	VML_WRITE32(VML_PIPEACONF, cur | VML_PIPE_FORCE_BORDER);
    } else {
	VML_WRITE32(VML_PIPEACONF, cur & ~VML_PIPE_FORCE_BORDER);
    }
    (void)VML_READ32(VML_PIPEACONF);
    return TRUE;

}

void
VERMILIONDisablePipe(ScrnInfoPtr pScrn)
{
    VERMILIONPtr pVermilion = VERMILIONPTR(pScrn);

    /* Disable the MDVO pad */
    VML_WRITE32(VML_RCOMPSTAT, 0);
    while (!(VML_READ32(VML_RCOMPSTAT) & VML_MDVO_VDC_I_RCOMP)) ;

    /* Disable display planes */
    VML_WRITE32(VML_DSPCCNTR, VML_READ32(VML_DSPCCNTR) & ~VML_GFX_ENABLE);
    (void)VML_READ32(VML_DSPCCNTR);
    /* Wait for vblank for the disable to take effect */
    VERMILIONWaitForVblank(pScrn);

    /* Next, disable display pipes */
    VML_WRITE32(VML_PIPEACONF, 0);
    (void)VML_READ32(VML_PIPEACONF);
}

void
VERMILIONDumpRegs(ScrnInfoPtr pScrn)
{
    VERMILIONPtr pVermilion = VERMILIONPTR(pScrn);
    ErrorF("Modesetting register dump:\n");
    ErrorF("\tHTOTAL_A         : 0x%08x\n", 
	   (unsigned) VML_READ32(VML_HTOTAL_A));
    ErrorF("\tHBLANK_A         : 0x%08x\n", 
	   (unsigned) VML_READ32(VML_HBLANK_A));
    ErrorF("\tHSYNC_A          : 0x%08x\n", 
	   (unsigned) VML_READ32(VML_HSYNC_A));
    ErrorF("\tVTOTAL_A         : 0x%08x\n", 
	   (unsigned) VML_READ32(VML_VTOTAL_A));
    ErrorF("\tVBLANK_A         : 0x%08x\n", 
	   (unsigned) VML_READ32(VML_VBLANK_A));
    ErrorF("\tVSYNC_A          : 0x%08x\n", 
	   (unsigned) VML_READ32(VML_VSYNC_A));
    ErrorF("\tDSPCSTRIDE       : 0x%08x\n", 
	   (unsigned) VML_READ32(VML_DSPCSTRIDE));
    ErrorF("\tDSPCSIZE         : 0x%08x\n", 
	   (unsigned) VML_READ32(VML_DSPCSIZE));
    ErrorF("\tDSPCPOS          : 0x%08x\n", 
	   (unsigned) VML_READ32(VML_DSPCPOS));
    ErrorF("\tDSPARB           : 0x%08x\n", 
	   (unsigned) VML_READ32(VML_DSPARB));
    ErrorF("\tDSPCADDR         : 0x%08x\n", 
	   (unsigned) VML_READ32(VML_DSPCADDR));
    ErrorF("\tBCLRPAT_A        : 0x%08x\n", 
	   (unsigned) VML_READ32(VML_BCLRPAT_A));
    ErrorF("\tCANVSCLR_A       : 0x%08x\n", 
	   (unsigned) VML_READ32(VML_CANVSCLR_A));
    ErrorF("\tPIPEASRC         : 0x%08x\n", 
	   (unsigned) VML_READ32(VML_PIPEASRC));
    ErrorF("\tPIPEACONF        : 0x%08x\n", 
	   (unsigned) VML_READ32(VML_PIPEACONF));
    ErrorF("\tDSPCCNTR         : 0x%08x\n", 
	   (unsigned) VML_READ32(VML_DSPCCNTR));
    ErrorF("\tRCOMPSTAT        : 0x%08x\n", 
	   (unsigned) VML_READ32(VML_RCOMPSTAT));
    ErrorF("End of modesetting register dump.\n");
}


/*
 * Sets the given video mode.
 */

Bool
VERMILIONDoSetMode(ScrnInfoPtr pScrn, DisplayModePtr pMode, Bool plane_enable)
{
    VERMILIONPtr pVermilion = VERMILIONPTR(pScrn);
    VERMILIONSys *sys = pVermilion->sys;
    CARD32 htot, hblank, hsync, vtot, vblank, vsync, dspcntr;
    CARD32 pipesrc, dspsize;
    int pixelClock;
    Bool ret = FALSE;
    int index;

    if (VERMILIONValidMode(pScrn->scrnIndex, pMode, FALSE, 0) != MODE_OK)
	goto done;

#if 0
    if (I830ModesEqual(&pVermilion->curMode, pMode))
	return TRUE;
#endif

    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Requested pix clock: %d\n",
	pMode->Clock);

    htot = (pMode->CrtcHDisplay - 1) | ((pMode->CrtcHTotal - 1) << 16);
    hblank =
	(pMode->CrtcHBlankStart - 1) | ((pMode->CrtcHBlankEnd - 1) << 16);
    hsync = (pMode->CrtcHSyncStart - 1) | ((pMode->CrtcHSyncEnd - 1) << 16);
    vtot = (pMode->CrtcVDisplay - 1) | ((pMode->CrtcVTotal - 1) << 16);
    vblank =
	(pMode->CrtcVBlankStart - 1) | ((pMode->CrtcVBlankEnd - 1) << 16);
    vsync = (pMode->CrtcVSyncStart - 1) | ((pMode->CrtcVSyncEnd - 1) << 16);
    pipesrc = ((pMode->HDisplay - 1) << 16) | (pMode->VDisplay - 1);
    dspsize = ((pMode->VDisplay - 1) << 16) | (pMode->HDisplay - 1);
    pixelClock = VERMILIONNearestClock(pScrn, pMode->Clock, &index);

    if (pVermilion->debug) {
	ErrorF
	    ("hact: %d htot: %d hbstart: %d hbend: %d hsyncstart: %d hsyncend: %d\n",
	     (int)(htot & 0xffff) + 1, (int)(htot >> 16) + 1,
	     (int)(hblank & 0xffff) + 1, (int)(hblank >> 16) + 1,
	     (int)(hsync & 0xffff) + 1, (int)(hsync >> 16) + 1);
	ErrorF
	    ("vact: %d vtot: %d vbstart: %d vbend: %d vsyncstart: %d vsyncend: %d\n",
	     (int)(vtot & 0xffff) + 1, (int)(vtot >> 16) + 1,
	     (int)(vblank & 0xffff) + 1, (int)(vblank >> 16) + 1,
	     (int)(vsync & 0xffff) + 1, (int)(vsync >> 16) + 1);
	ErrorF("pipesrc: %dx%d, dspsize: %dx%d\n", (int)(pipesrc >> 16) + 1,
	       (int)(pipesrc & 0xffff) + 1, (int)(dspsize & 0xffff) + 1,
	       (int)(dspsize >> 16) + 1);
	ErrorF("Actual Pixel clock is %d kHz\n"
	       "\t Horizontal frequency is %.1f kHz\n"
	       "\t Vertical frequency is %.1f Hz\n",
	       pixelClock,
	       (float)pixelClock / (float)(pMode->CrtcHTotal),
	       (float)pixelClock / (float)(pMode->CrtcHTotal) /
	       (float)(pMode->CrtcVTotal) * 1000.);
    }
    dspcntr = VML_GFX_ENABLE | VML_GFX_GAMMABYPASS;
    switch (pScrn->depth) {
    case 15:
	dspcntr |= VML_GFX_ARGB1555;
	break;
    case 24:
	dspcntr |= VML_GFX_RGB0888;
	break;
    default:
	ErrorF("Unknown display BPP\n");
	goto done;
    }

    /* Finally, set the mode. */
    VERMILIONDisablePipe(pScrn);
    mem_barrier();

    /* Set pixel clock */
    if (!sys->setClock(sys, pixelClock)) 
	return FALSE;

    VML_WRITE32(VML_HTOTAL_A, htot);
    VML_WRITE32(VML_HBLANK_A, hblank);
    VML_WRITE32(VML_HSYNC_A, hsync);
    VML_WRITE32(VML_VTOTAL_A, vtot);
    VML_WRITE32(VML_VBLANK_A, vblank);
    VML_WRITE32(VML_VSYNC_A, vsync);
    VML_WRITE32(VML_DSPCSTRIDE, pVermilion->stride);
    VML_WRITE32(VML_DSPCSIZE, dspsize);
    VML_WRITE32(VML_DSPCPOS, 0x00000000);
    VML_WRITE32(VML_DSPARB, VML_FIFO_DEFAULT);
    /* Black border color */
    VML_WRITE32(VML_BCLRPAT_A, 0x00000000);
    /* Black canvas color */
    VML_WRITE32(VML_CANVSCLR_A, 0x00000000);
    VML_WRITE32(VML_PIPEASRC, pipesrc);
    (void)VML_READ32(VML_PIPEASRC);
    mem_barrier();

    /* Then, turn the pipe on first. */
    VML_WRITE32(VML_PIPEACONF, VML_PIPE_ENABLE);
    (void)VML_READ32(VML_PIPEACONF);
    mem_barrier();

    VML_WRITE32(VML_DSPCCNTR, dspcntr);
    VERMILIONSetGraphicsOffset(pScrn, pVermilion->x, pVermilion->y);
    
    /* Enable the MDVO pad */
    VML_WRITE32(VML_RCOMPSTAT, VML_MDVO_PAD_ENABLE);
    while (!(VML_READ32(VML_RCOMPSTAT) &
	    (VML_MDVO_VDC_I_RCOMP | VML_MDVO_PAD_ENABLE))) ;

    pVermilion->curMode = *pMode;
    if (pVermilion->debug)
	VERMILIONDumpRegs(pScrn);
    ret = TRUE;
  done:
    return ret;
}
