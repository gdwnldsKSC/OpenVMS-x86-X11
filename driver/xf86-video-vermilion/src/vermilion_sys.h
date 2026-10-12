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

#ifndef _VERMILION_SYS_H_
#define _VERMILION_SYS_H_

struct _VERMILIONSysPriv;

typedef struct _VERMILIONSys
{

    /*
     * Subsystem identifier:
     */

    void (*subSys) (struct _VERMILIONSys * sys,
	const char **name, unsigned *id0, unsigned *id1);
    /*
     * Save / Restore;
     */

    int (*save) (struct _VERMILIONSys * sys);
    int (*restore) (struct _VERMILIONSys * sys);

    /*
     * PLL programming;
     */

    void (*destroy) (struct _VERMILIONSys * sys);
        Bool(*progClock) (struct _VERMILIONSys * sys);
    void (*clockRanges) (const struct _VERMILIONSys * sys, int *low,
	int *high);
    void (*clocks) (const struct _VERMILIONSys * sys, int *numClocks,
	int clocks[]);
        Bool(*setClock) (struct _VERMILIONSys * sys, int clock);

    /*
     * Panel type and functions.
     */

    int (*panel) (const struct _VERMILIONSys * sys);
    void (*panelOn) (const struct _VERMILIONSys * sys);
    void (*panelOff) (const struct _VERMILIONSys * sys);
    void (*backlightOn) (const struct _VERMILIONSys * sys);
    void (*backlightOff) (const struct _VERMILIONSys * sys);

    /*
     * Private information.
     */
    int id;
    pointer *priv;
} VERMILIONSys;

VERMILIONSys *VERMILIONCreateSys(ScrnInfoPtr pScrn);

#endif
