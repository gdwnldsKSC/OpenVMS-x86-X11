/*
 * Copyright (c) 2026 Kisara Development LLC
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* The standard fbdevhw interface for the firmware's existing framebuffer.
 * libpciaccess owns discovery, physical mappings and the display lease.
 * The upstream fbdev driver owns all rendering, through its shadow buffer.
 * No register programming, firmware calls or Linux framebuffer emulation.
 */
#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xf86.h"
#include "xf86Modes.h"
#include "xf86cmap.h"
#include "fbdevhw.h"
#include "openvms_bootfb.h"

static XF86ModuleVersionInfo fbdevHWVersRec = {
    "fbdevhw", MODULEVENDORSTRING, MODINFOSTRING1, MODINFOSTRING2,
    XORG_VERSION_CURRENT, 0, 0, 2,
    ABI_CLASS_VIDEODRV, ABI_VIDEODRV_VERSION, MOD_CLASS_NONE, {0, 0, 0, 0}
};
_X_EXPORT XF86ModuleData fbdevhwModuleData = { &fbdevHWVersRec, NULL, NULL };

typedef struct {
    struct openvms_bootfb info;
    DisplayModeRec mode;
    void *pixels;
    uint64_t mapped_bytes;
    xf86FreeScreenProc *FreeScreen;
} BootFBRec, *BootFBPtr;

static int private_index = -1;
static char framebuffer_name[] = "OpenVMS firmware framebuffer";

static BootFBPtr
bootfb(ScrnInfoPtr pScrn)
{
    return private_index < 0 ? NULL : pScrn->privates[private_index].ptr;
}

static Bool
bootfb_error(ScrnInfoPtr pScrn, const char *message)
{
    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "fbdevhw: %s\n", message);
    return FALSE;
}

static Bool
bootfb_stride(const char *text, uint32_t *stride)
{
    uint64_t value = 0;
    if (!text || !*text)
        return FALSE;
    for (; *text; ++text) {
        if (*text < '0' || *text > '9')
            return FALSE;
        value = value * 10 + (*text - '0');
        if (value > INT_MAX)
            return FALSE;
    }
    *stride = (uint32_t)value;
    return value != 0;
}

/* The Display subsection is selected after fbdevHWInit, by SetDepthBpp. */
static Bool
bootfb_layout(ScrnInfoPtr pScrn)
{
    BootFBPtr p = bootfb(pScrn);
    DispPtr display = pScrn->display;
    const char **name;
    char geometry[32];
    if (!p || !display)
        return FALSE;
    if (pScrn->depth != 24 || pScrn->bitsPerPixel != 32 ||
        pScrn->defaultVisual != TrueColor)
        return bootfb_error(pScrn, "requires depth 24, 32 bits per pixel and TrueColor");
    if ((display->virtualX && display->virtualX != (int)p->info.width) ||
        (display->virtualY && display->virtualY != (int)p->info.height) ||
        pScrn->virtualX != (int)p->info.width ||
        pScrn->virtualY != (int)p->info.height ||
        pScrn->displayWidth != (int)p->info.width ||
        display->frameX0 > 0 || display->frameY0 > 0)
        return bootfb_error(pScrn, "Virtual and viewport must match the firmware framebuffer");
    snprintf(geometry, sizeof(geometry), "%ux%u", p->info.width, p->info.height);
    for (name = display->modes; name && *name; ++name)
        if (strcmp(*name, "current") && strcmp(*name, geometry))
            return bootfb_error(pScrn, "only the current firmware mode is supported");
    return TRUE;
}

static void
bootfb_free_screen(ScrnInfoPtr pScrn)
{
    BootFBPtr p = bootfb(pScrn);
    xf86FreeScreenProc *previous = p->FreeScreen;
    if (!fbdevHWUnmapVidmem(pScrn))
        FatalError("OpenVMS fbdevhw cannot release its framebuffer mapping\n");
    pScrn->privates[private_index].ptr = NULL;
    pScrn->FreeScreen = previous;
    free(p);
    if (previous)
        previous(pScrn);
}

Bool
fbdevHWProbe(struct pci_device *pPci, char *device, char **namep)
{
    struct openvms_bootfb info;
    if (namep)
        *namep = NULL;
    /* HWRPB identifies the boot framebuffer, not a particular PCI function. */
    if (pPci || !device || strcmp(device, "HWRPB") ||
        openvms_bootfb_query(&info))
        return FALSE;
    if (namep) {
        *namep = strdup(framebuffer_name);
        if (!*namep)
            return FALSE;
    }
    return TRUE;
}

Bool
fbdevHWInit(ScrnInfoPtr pScrn, struct pci_device *pPci, char *device)
{
    struct openvms_bootfb info;
    EntityInfoPtr entity;
    XF86OptionPtr options;
    const char *format;
    BootFBPtr p;
    int error;
    if (pPci || !device || strcmp(device, "HWRPB") || pScrn->numEntities != 1)
        return bootfb_error(pScrn, "requires Option fbdev HWRPB and a framebuffer entity");
    error = openvms_bootfb_query(&info);
    if (error) {
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "fbdevhw: query: %s\n", strerror(error));
        return FALSE;
    }
    entity = xf86GetEntityInfo(pScrn->entityList[0]);
    if (!entity)
        return FALSE;
    options = entity->device ? entity->device->options : NULL;
    free(entity);
    format = xf86FindOptionValue(options, "PixelFormat");
    if (!bootfb_stride(xf86FindOptionValue(options, "StrideBytes"), &info.stride_bytes) ||
        !format || (strcmp(format, "XRGB8888") && strcmp(format, "XBGR8888")))
        return bootfb_error(pScrn, "requires verified StrideBytes (decimal) and PixelFormat XRGB8888 or XBGR8888");
    info.pixel_format = !strcmp(format, "XRGB8888") ?
        OPENVMS_BOOTFB_XRGB8888 : OPENVMS_BOOTFB_XBGR8888;
    if (!info.width || !info.height || info.width > 32767 || info.height > 32767 ||
        (info.stride_bytes & 3) || info.stride_bytes < (uint64_t)info.width * 4 ||
        (uint64_t)info.stride_bytes * info.height > info.byte_length ||
        (uint64_t)info.stride_bytes * info.height > INT_MAX)
        return bootfb_error(pScrn, "framebuffer geometry, stride or allocation bounds are invalid");
    if (!xf86SetBoolOption(options, "ShadowFB", TRUE) ||
        xf86FindOptionValue(options, "Rotate"))
        return bootfb_error(pScrn, "requires ShadowFB with no Rotate option");
    xf86MarkOptionUsedByName(options, "fbdev");
    xf86MarkOptionUsedByName(options, "StrideBytes");
    xf86MarkOptionUsedByName(options, "PixelFormat");
    if (private_index < 0)
        private_index = xf86AllocateScrnInfoPrivateIndex();
    if (bootfb(pScrn))
        return bootfb_error(pScrn, "framebuffer is already initialized");
    p = calloc(1, sizeof(*p));
    if (!p)
        return FALSE;
    p->info = info;
    p->mapped_bytes = (uint64_t)info.stride_bytes * info.height;
    p->mode.name = "current";
    p->mode.type = M_T_BUILTIN;
    p->mode.status = MODE_OK;
    /* HWRPB has no timing information. These delimit the visible rectangle;
     * a zero clock makes no claim about refresh rate or programmed timings. */
    p->mode.HDisplay = p->mode.HSyncStart = p->mode.HSyncEnd = p->mode.HTotal = info.width;
    p->mode.VDisplay = p->mode.VSyncStart = p->mode.VSyncEnd = p->mode.VTotal = info.height;
    p->mode.CrtcHDisplay = info.width;
    p->mode.CrtcVDisplay = info.height;
    p->FreeScreen = pScrn->FreeScreen;
    pScrn->FreeScreen = bootfb_free_screen;
    pScrn->privates[private_index].ptr = p;
    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
               "fbdevhw: fixed %ux%u, stride %u, %s; no hardware mode changes, panning, palette or DPMS\n",
               info.width, info.height, info.stride_bytes, format);
    return TRUE;
}

char *fbdevHWGetName(ScrnInfoPtr pScrn) { (void)pScrn; return framebuffer_name; }
int fbdevHWGetDepth(ScrnInfoPtr pScrn, int *bpp)
{ (void)pScrn; if (bpp) *bpp = 32; return 24; }
int fbdevHWGetType(ScrnInfoPtr pScrn) { (void)pScrn; return FBDEVHW_PACKED_PIXELS; }
int fbdevHWGetLineLength(ScrnInfoPtr pScrn) { return bootfb(pScrn)->info.stride_bytes; }
int fbdevHWGetVidmem(ScrnInfoPtr pScrn) { return (int)bootfb(pScrn)->mapped_bytes; }
int fbdevHWLinearOffset(ScrnInfoPtr pScrn) { (void)pScrn; return 0; }

void *
fbdevHWMapVidmem(ScrnInfoPtr pScrn)
{
    BootFBPtr p = bootfb(pScrn);
    int error;
    if (!bootfb_layout(pScrn))
        return NULL;
    if (!p->pixels) {
        error = openvms_bootfb_map(&p->info, &p->pixels);
        if (error) {
            xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "fbdevhw: map: %s\n", strerror(error));
            return NULL;
        }
    }
    return p->pixels;
}

Bool
fbdevHWUnmapVidmem(ScrnInfoPtr pScrn)
{
    BootFBPtr p = bootfb(pScrn);
    int error;
    if (!p || !p->pixels)
        return TRUE;
    error = openvms_bootfb_unmap(p->pixels, p->mapped_bytes);
    if (error) {
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "fbdevhw: unmap: %s\n", strerror(error));
        return FALSE;
    }
    p->pixels = NULL;
    pScrn->vtSema = FALSE;
    return TRUE;
}

void fbdevHWSetVideoModes(ScrnInfoPtr pScrn) { (void)pScrn; }

void
fbdevHWUseBuildinMode(ScrnInfoPtr pScrn)
{
    BootFBPtr p = bootfb(pScrn);
    /* Xorg owns and frees this copy during normal screen deletion. */
    pScrn->modes = xf86DuplicateMode(&p->mode);
    pScrn->modes->next = pScrn->modes->prev = pScrn->modes;
    pScrn->virtualX = p->info.width;
    pScrn->virtualY = p->info.height;
}

ModeStatus
fbdevHWValidMode(ScrnInfoPtr pScrn, DisplayModePtr mode, Bool verbose, int flags)
{
    BootFBPtr p = bootfb(pScrn);
    (void)verbose;
    (void)flags;
    return p && mode && xf86ModesEqual(mode, &p->mode) ? MODE_OK : MODE_BAD;
}

Bool
fbdevHWModeInit(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    BootFBPtr p = bootfb(pScrn);
    if (!p || !p->pixels || !bootfb_layout(pScrn) ||
        fbdevHWValidMode(pScrn, mode, FALSE, 0) != MODE_OK)
        return FALSE;
    pScrn->offset.red = p->info.pixel_format == OPENVMS_BOOTFB_XRGB8888 ? 16 : 0;
    pScrn->offset.green = 8;
    pScrn->offset.blue = 16 - pScrn->offset.red;
    pScrn->mask.red = 0xffU << pScrn->offset.red;
    pScrn->mask.green = 0xff00U;
    pScrn->mask.blue = 0xffU << pScrn->offset.blue;
    pScrn->weight.red = pScrn->weight.green = pScrn->weight.blue = 8;
    pScrn->vtSema = TRUE;
    return TRUE;
}

Bool fbdevHWSwitchMode(ScrnInfoPtr pScrn, DisplayModePtr mode)
{ return fbdevHWModeInit(pScrn, mode); }
void fbdevHWSave(ScrnInfoPtr pScrn) { (void)pScrn; }
void fbdevHWRestore(ScrnInfoPtr pScrn) { pScrn->vtSema = FALSE; }

void
fbdevHWAdjustFrame(ScrnInfoPtr pScrn, int x, int y)
{
    if (x || y)
        bootfb_error(pScrn, "panning is unsupported; viewport remains at 0,0");
}

Bool
fbdevHWEnterVT(ScrnInfoPtr pScrn)
{
    return !pScrn->frameX0 && !pScrn->frameY0 &&
           fbdevHWModeInit(pScrn, pScrn->currentMode);
}

/* fbdev caches the mapped address: retain it until UnmapVidmem/FreeScreen. */
void fbdevHWLeaveVT(ScrnInfoPtr pScrn) { fbdevHWRestore(pScrn); }
void fbdevHWLoadPalette(ScrnInfoPtr pScrn, int n, int *indices, LOCO *colors, VisualPtr visual)
{ (void)pScrn; (void)n; (void)indices; (void)colors; (void)visual; }
void fbdevHWDPMSSet(ScrnInfoPtr pScrn, int mode, int flags)
{ (void)pScrn; (void)mode; (void)flags; }
Bool fbdevHWSaveScreen(ScreenPtr screen, int mode)
{ (void)screen; return xf86IsUnblank(mode); }
