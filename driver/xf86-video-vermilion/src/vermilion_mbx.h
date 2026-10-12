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
 * Authors: Alan Hourihane <alanh@tungstengraphics.com>
 *	    Michel Dänzer <michel@tungstengraphics.com>
 */

#ifndef _VERMILION_MBX_H_
#define _VERMILION_MBX_H_

#ifndef XFree86LOADER
#include <sys/mman.h>
#endif

#define ReadHWReg(ui32Offset) *(volatile CARD32*)((char*)pVermilion->mbxRegsBase+ui32Offset)

#define MBX1_GLOBREG_INT_STATUS                 0x012C
#define MBX1_INT_TA_FREEVCOUNT_MASK             0x00FF0000
#define MBX1_INT_TA_FREEVCOUNT_SHIFT    16

/*
	MBX Slave Port's offset into the register aperture
*/
#define MBX_SP_2D_SYS_PHYS_OFFSET		0xA00000

/*
        MBX Slave Port FIFO Size (in units of `Bits per Write Bus Width')
        Includes 5 slot safety factor for fullness register latency
*/
#define MBX1_SP_FIFO_DWSIZE     123

/*
        Macro to extract FIFO space from HW register value
*/
#define MBX_EXTRACT_FIFO_COUNT(x)   (((x) & MBX1_INT_TA_FREEVCOUNT_MASK) >> MBX1_INT_TA_FREEVCOUNT_SHIFT)

static __inline__ void
MBXAcquireFifoSpace(VERMILIONPtr pVermilion, CARD32 n)
{
    while (pVermilion->FifoSlots < n) {
	/* read fifo space from HW */
	pVermilion->FifoSlots = (MBX1_SP_FIFO_DWSIZE -
	    MBX_EXTRACT_FIFO_COUNT(ReadHWReg(MBX1_GLOBREG_INT_STATUS)));
    }
}

static __inline__ void
MBXWriteSlavePortBatch(VERMILIONPtr pVermilion, void *pvLinDataAddr,
    CARD32 ui32DWORDs)
{
    CARD32 *pui32LinDataAddr = (CARD32 *) pvLinDataAddr;
    CARD32 *pui32LinPortAddrBase = pVermilion->slavePort;

    /* write to the slaveport */
    while (ui32DWORDs--) {
	*pui32LinPortAddrBase = *pui32LinDataAddr++;
    }
}

#define WRITESLAVEPORTDATA(n)			\
	(void) MBXWriteSlavePortBatch ( pVermilion, auBltPacket, n );

#define WAITFIFO(n) 		\
	MBXAcquireFifoSpace(pVermilion, n);	\
	pVermilion->FifoSlots -= n;

/*
 * Block headers
 */
#define	MBX2D_CTRL_BH		0x20000000
#define	MBX2D_SRC_OFF_BH	0x30000000
#define	MBX2D_FENCE_BH		0x70000000	/* Flush between two blits */
#define	MBX2D_BLIT_BH		0x80000000
#define	MBX2D_SRC_CTRL_BH	0x90000000
#define MBX2D_DST_CTRL_BH	0xA0000000

#define MBX2D_USE_PAT			0x00010000

#define MBX2D_TEXTCOPY_TL2BR	0x00000000
#define MBX2D_TEXTCOPY_TR2BL	0x00800000
#define MBX2D_TEXTCOPY_BL2TR	0x01000000

#define MBX2D_SRCCK_REJECT		0x00100000
#define MBX2D_SRCCK_CTRL	0x00000001

#define MBX2D_SRC_FBMEM				0x04000000
#define MBX2D_SRC_555RGB			0x00040000
#define MBX2D_SRC_8888ARGB			0x00060000

#endif /* _VERMILION_MBX_H_ */
