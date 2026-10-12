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
#ifndef _VERMILION_KERNEL_H_
#define _VERMILION_KERNEL_H_

#ifdef __KERNEL__
#include<linux/ioctl.h>
#include<linux/types.h>
#else
#include <stdint.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#define __u64 uint64_t
#endif

#define VML_KI_MAJOR 2
#define VML_KI_MINOR 0

typedef struct {
	unsigned bus;
	unsigned slot;
	unsigned function;
} vml_pci_tag;

typedef struct {
	vml_pci_tag gpu_tag;
	__u64 vram_offset;
	__u64 vram_contig_size;
	__u64 vram_total_size;
} vml_init_rep_t;

typedef struct {
	unsigned major;
	unsigned minor;
	vml_pci_tag vdc_tag;
        unsigned pipe;
} vml_init_req_t;

typedef union {
	vml_init_req_t req;
	vml_init_rep_t rep;
} vml_init_t;

#define VML_IOC_MAGIC 0xD0
#define VML_INIT_DEVICE _IOWR(VML_IOC_MAGIC, 0, vml_init_t)
#define VML_IOC_MAXNR 0

#endif
