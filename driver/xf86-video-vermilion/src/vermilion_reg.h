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
 * Authors: Thomas Hellström <thomas-at-tungstengraphics-dot-com>
 */

#ifndef _VERMILION_REG_H_
#define _VERMILION_REG_H_

#define VML_READ32(_offs) \
  (*((volatile CARD32 *) ((volatile CARD8 *)pVermilion->vdcRegsBase + (_offs))))
#define VML_WRITE32(__offs, _data) \
  (VML_READ32(__offs) = (_data));

/*
 * Display controller registers:
 */

/* Display controller 10-bit color representation */

#define VML_R_MASK                   0x3FF00000
#define VML_R_SHIFT                  20
#define VML_G_MASK                   0x000FFC00
#define VML_G_SHIFT                  10
#define VML_B_MASK                   0x000003FF
#define VML_B_SHIFT                  0

/* Graphics plane control */
#define VML_DSPCCNTR                 0x00072180
#define VML_GFX_ENABLE               0x80000000
#define VML_GFX_GAMMABYPASS          0x40000000
#define VML_GFX_ARGB1555             0x0C000000
#define VML_GFX_RGB0888              0x18000000
#define VML_GFX_ARGB8888             0x1C000000
#define VML_GFX_ALPHACONST           0x00800000
#define VML_GFX_CONST_ALPHA          0x000000FF

/* Graphics plane start address. Pixel aligned. */
#define VML_DSPCADDR                 0x00072184

/* Graphics plane stride register. */
#define VML_DSPCSTRIDE               0x00072188

/* Graphics plane position register. */
#define VML_DSPCPOS                  0x0007218C
#define VML_POS_YMASK                0x0FFF0000
#define VML_POS_YSHIFT               16
#define VML_POS_XMASK                0x00000FFF
#define VML_POS_XSHIFT               0

/* Graphics plane height and width */
#define VML_DSPCSIZE                 0x00072190
#define VML_SIZE_HMASK               0x0FFF0000
#define VML_SIZE_HSHIFT              16
#define VML_SISE_WMASK               0x00000FFF
#define VML_SIZE_WSHIFT              0

/* Graphics plane gamma correction lookup table registers (129 * 32 bits) */
#define VML_DSPCGAMLUT               0x00072200

/* Pixel video output configuration register */
#define VML_PVOCONFIG                0x00061140
#define VML_CONFIG_BASE              0x80000000
#define VML_CONFIG_PIXEL_SWAP        0x04000000
#define VML_CONFIG_DE_INV            0x01000000
#define VML_CONFIG_HREF_INV          0x00400000
#define VML_CONFIG_VREF_INV          0x00100000
#define VML_CONFIG_CLK_INV           0x00040000
#define VML_CONFIG_CLK_DIV2          0x00010000
#define VML_CONFIG_ESTRB_INV         0x00008000

/* Pipe A Horizontal total register */
#define VML_HTOTAL_A                 0x00060000
#define VML_HTOTAL_MASK              0x1FFF0000
#define VML_HTOTAL_SHIFT             16
#define VML_HTOTAL_VAL               8192
#define VML_HACTIVE_MASK             0x000007FF
#define VML_HACTIVE_SHIFT            0
#define VML_HACTIVE_VAL              4096

/* Pipe A Horizontal Blank register */
#define VML_HBLANK_A                 0x00060004
#define VML_HBLANK_END_MASK          0x1FFF0000
#define VML_HBLANK_END_SHIFT         16
#define VML_HBLANK_END_VAL           8192
#define VML_HBLANK_START_MASK        0x00001FFF
#define VML_HBLANK_START_SHIFT       0
#define VML_HBLANK_START_VAL         8192

/* Pipe A Horizontal Sync register */
#define VML_HSYNC_A                  0x00060008
#define VML_HSYNC_END_MASK           0x1FFF0000
#define VML_HSYNC_END_SHIFT          16
#define VML_HSYNC_END_VAL            8192
#define VML_HSYNC_START_MASK         0x00001FFF
#define VML_HSYNC_START_SHIFT        0
#define VML_HSYNC_START_VAL          8192

/* Pipe A Vertical total register */
#define VML_VTOTAL_A                 0x0006000C
#define VML_VTOTAL_MASK              0x1FFF0000
#define VML_VTOTAL_SHIFT             16
#define VML_VTOTAL_VAL               8192
#define VML_VACTIVE_MASK             0x000007FF
#define VML_VACTIVE_SHIFT            0
#define VML_VACTIVE_VAL              4096

/* Pipe A Vertical Blank register */
#define VML_VBLANK_A                 0x00060010
#define VML_VBLANK_END_MASK          0x1FFF0000
#define VML_VBLANK_END_SHIFT         16
#define VML_VBLANK_END_VAL           8192
#define VML_VBLANK_START_MASK        0x00001FFF
#define VML_VBLANK_START_SHIFT       0
#define VML_VBLANK_START_VAL         8192

/* Pipe A Vertical Sync register */
#define VML_VSYNC_A                  0x00060014
#define VML_VSYNC_END_MASK           0x1FFF0000
#define VML_VSYNC_END_SHIFT          16
#define VML_VSYNC_END_VAL            8192
#define VML_VSYNC_START_MASK         0x00001FFF
#define VML_VSYNC_START_SHIFT        0
#define VML_VSYNC_START_VAL          8192

/* Pipe A Source Image size (minus one - equal to active size)
 * Programmable while pipe is enabled.
 */
#define VML_PIPEASRC                 0x0006001C
#define VML_PIPEASRC_HMASK           0x0FFF0000
#define VML_PIPEASRC_HSHIFT          16
#define VML_PIPEASRC_VMASK           0x00000FFF
#define VML_PIPEASRC_VSHIFT          0

/* Pipe A Border Color Pattern register (10 bit color) */
#define VML_BCLRPAT_A                0x00060020

/* Pipe A Canvas Color register  (10 bit color) */
#define VML_CANVSCLR_A               0x00060024

/* Pipe A Configuration register */
#define VML_PIPEACONF                0x00070008
#define VML_PIPE_BASE                0x00000000
#define VML_PIPE_ENABLE              0x80000000
#define VML_PIPE_FORCE_BORDER        0x02000000
#define VML_PIPE_PLANES_OFF          0x00080000
#define VML_PIPE_ARGB_OUTPUT_MODE    0x00040000

/* Pipe A FIFO setting */
#define VML_DSPARB                   0x00070030
#define VML_FIFO_DEFAULT             0x00001D9C

/* MDVO rcomp status & pads control register */
#define VML_RCOMPSTAT                0x00070048
#define VML_MDVO_VDC_I_RCOMP         0x80000000
#define VML_MDVO_POWERSAVE_OFF       0x00000008
#define VML_MDVO_PAD_ENABLE          0x00000004
#define VML_MDVO_PULLDOWN_ENABLE     0x00000001

#endif
