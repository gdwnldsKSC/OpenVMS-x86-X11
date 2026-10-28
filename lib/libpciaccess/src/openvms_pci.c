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

/*
 * Native OpenVMS x86-64 discovery, bounded VMware VGA resource access,
 * and boot-framebuffer access through the same display lease and PFN mapper.
 *
 * References: OpenVMS AXP Device Support Developer's Guide AA-Q28SA-TE, 
 * sections 13.5-13.13; Device Support Reference, AA-Q28PA-TE,
 * ADP/BUSARRAY and SCH_STD$IOLOCKR/IOUNLOCK; current VSI System Services,
 * RTL Library and System Analysis manuals.  
 *
 * The executive interfaces require CMKRNL privilege and LINK/SYSEXE.  
 * Enumeration remains read-only. The operator must explicitly select one
 * VMware VGA function and assert exclusive access before resource probing:
 * PCIACCESS_OPENVMS_DEVICE=domain:bus:device.function (hexadecimal), and
 * PCIACCESS_OPENVMS_EXCLUSIVE=1. The latter asserts that OPA0 is serial and
 * no native driver owns video. A cooperating process lock cannot prove this.
 *
 * BAR sizes are measured through IOC, never guessed from SVGA registers.
 * This initial x86 backend supports assigned 32-bit BARs only, identity PCI
 * memory addresses, and uncached mappings. No relocation or ROM execution.
 * PFN services: https://docs.vmssoftware.com/
 * vsi-openvms-system-services-reference-manual-a-getuai/
 */

#ifndef __NEW_STARLET
#define __NEW_STARLET 1
#endif
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <adpdef.h>
#include <busarraydef.h>
#include <busdef.h>
#include <descrip.h>
#include <gen64def.h>
#include <hwrpbdef.h>
#include <iocdef.h>
#include <ioc_routines.h>
#include <ipldef.h>
#include <lib$routines.h>
#include <lckdef.h>
#include <pcbdef.h>
#include <pcidef.h>
#include <psldef.h>
#include <sch_routines.h>
#include <secdef.h>
#include <ssdef.h>
#include <starlet.h>
#include <syidef.h>
#include <vadef.h>
#include <vms_macros.h>

#include "pciaccess.h"
#include "pciaccess_private.h"
#include "openvms_pci_private.h"
#include "openvms_pci_resource.h"
#include "openvms_pci_port.h"
#include "openvms_bootfb.h"

#if !defined(__VMS) || (!defined(__x86_64) && !defined(__x86_64__))
#error This backend requires native OpenVMS x86-64.
#endif
#if !defined(__INITIAL_POINTER_SIZE) || __INITIAL_POINTER_SIZE != 64
#error Compile this backend with /POINTER_SIZE=64.
#endif

#ifndef ENOTSUP
#define ENOTSUP ENOSYS
#endif

#pragma __required_pointer_size __save
#pragma __required_pointer_size __short
typedef ADP *vms_adp32;
typedef PCB *vms_pcb32;
typedef int *vms_int32;
#pragma extern_model save
#pragma extern_model strict_refdef
extern vms_adp32 IOC$GL_ADPLIST;
extern vms_pcb32 CTL$GL_PCB;
#pragma extern_model restore
#pragma __required_pointer_size __restore

#pragma __required_pointer_size __save
#pragma __required_pointer_size __long
#pragma extern_model save
#pragma extern_model strict_refdef
extern HWRPB *EXE$GPQ_HWRPB;
#pragma extern_model restore
#pragma __required_pointer_size __restore

/* Fail closed on malformed/cyclic executive inventories, without allocating
 * or printing while holding the I/O database mutex. */
enum {
    VMS_MAX_ADAPTERS = 1024,
    VMS_MAX_BUS_ENTRIES = 1024,
    VMS_MAX_DEVICES = 1024,
    VMS_CONFIG_BYTES = 256
};

struct vms_system {
    struct pci_system base;
    struct vms_device *native;
    int unlock_image;
    struct pci_device *selected;
    struct vms_bar_state video;
    struct { uint16_t status, reserved; uint32_t id; } lock;
    struct vms_mapping *maps;
    struct pci_io_handle *io[16];
};

struct vms_request {
    int inventory;
    int error;
    struct vms_device *devices;
    size_t count;
    struct vms_device target;
    unsigned int offset;
    unsigned int size;
    unsigned int completed;
    unsigned char *data;
    unsigned int operation;
    struct vms_bar_state *video;
    struct openvms_bootfb bootfb;
};

enum { VMS_READ, VMS_PROBE, VMS_WRITE, VMS_BOOTFB };

struct vms_mapping {
    struct vms_mapping *next;
    GENERIC_64 region;
    void *base, *address;
    unsigned __int64 length;
    pciaddr_t requested;
};

static int vms_video_request(vms_adp32, struct vms_request *);

static int
vms_status_error(unsigned int status)
{
    if (status & 1)
        return 0;
    if (status == SS$_NOPRIV || status == SS$_NOCMKRNL)
        return EPERM;
    return EIO;
}

/* Only aligned longword reads are used.  This avoids historical differences
 * in the IOC byte/word lane conventions.  IOC owns machine-check locking. */
static int
vms_config_long(vms_adp32 adapter, uint64_t node_number, unsigned int offset,
                uint32_t *value)
{
    PCI_NODE_NUMBER node;
    int result = 0;
    uintptr_t address = (uintptr_t)&result;
    unsigned int status;

    /* IOC's current node argument is a longword.  Segment selection through
     * ADP is not a validated public contract here: reject it before any I/O,
     * rather than truncate the full node and read a different function. */
    node.pci$q_pci_node_number_q = node_number;
    if (node.pci$v_pci_node_number_segment ||
        node.pci$v_pci_node_number_offset ||
        node.pci$v_pci_node_number_reserved)
        return ENOTSUP;
    /* IOC has a pointer32 output argument.  Never truncate a kernel-stack
     * address if a future OS/compiler changes its placement. */
    if ((uintptr_t)(intptr_t)(int32_t)address != address)
        return ENOTSUP;
    status = ioc$read_pci_config(adapter, node.pci$l_pci_node_number,
                                 (int)offset, 4,
                                 (vms_int32)&result);
    if (!(status & 1))
        return vms_status_error(status);
    *value = (uint32_t)result;
    return 0;
}

static int
vms_observe(vms_adp32 adapter, BUSARRAYENTRY *entry,
            struct vms_device *device)
{
    int error;
    unsigned int type;

    device->tr = adapter->adp$l_tr;
    device->node = entry->busarray$q_node_number;
    error = vms_config_long(adapter, device->node, 0,
                            &device->id);
    if (error)
        return error;
    if ((device->id & 0xffffu) == 0 || (device->id & 0xffffu) == 0xffffu)
        return ENODEV;
    error = vms_config_long(adapter, device->node, 8,
                            &device->class_revision);
    if (error)
        return error;
    error = vms_config_long(adapter, device->node, 12,
                            &device->header);
    if (error)
        return error;
    type = (device->header >> 16) & 0x7fu;
    device->subsystem = 0;
    if (type == 0 || type == 2) {
        error = vms_config_long(adapter, device->node,
                                type == 0 ? 0x2c : 0x40,
                                &device->subsystem);
        if (error)
            return error;
    }
    device->inventory_matches =
        (uint32_t)entry->busarray$q_hw_id == device->id;
    return 0;
}

static int
vms_read_entry(vms_adp32 adapter, struct vms_request *request)
{
    uint32_t value;
    unsigned int position;
    int error;

    error = vms_config_long(adapter, request->target.node, 0,
                            &value);
    if (error)
        return error;
    if (value != request->target.id)
        return ENODEV;
    if (request->operation != VMS_READ)
        return vms_video_request(adapter, request);
    while (request->completed < request->size) {
        position = request->offset + request->completed;
        error = vms_config_long(adapter, request->target.node,
                                position & ~3u, &value);
        if (error)
            return error;
        do {
            request->data[request->completed++] =
                (unsigned char)(value >> ((position & 3u) * 8));
            position++;
        } while ((position & 3u) && request->completed < request->size);
    }
    return 0;
}

/* Read only the OS-owned restart block through installed definitions. This
 * path needs no PCI access, I/O database lock, firmware call or IPL change.
 * Pitch and pixel format are not established by these HWRPB fields.
 */
static int
vms_kernel_bootfb(struct openvms_bootfb *info)
{
    const volatile HWRPB *restart = EXE$GPQ_HWRPB;
    uintptr_t address = (uintptr_t)restart;
    uint64_t extent, physical, length;
    uint32_t width, height;

    if (!restart) return ENODEV;
    if ((address >> 47) != UINT64_C(0x1ffff) || (address & 15u))
        return ENOTSUP;
    extent = restart->hwrpb$iq_size;
#define BOOTFB_FIELD_PRESENT(member) \
    (extent >= offsetof(HWRPB, member) + sizeof(restart->member))
    if (!BOOTFB_FIELD_PRESENT(hwrpb$iq_ident) ||
        !BOOTFB_FIELD_PRESENT(hwrpb$iq_framebuffer_pa) ||
        !BOOTFB_FIELD_PRESENT(hwrpb$iq_framebuffer_size) ||
        !BOOTFB_FIELD_PRESENT(hwrpb$il_horiz_rez) ||
        !BOOTFB_FIELD_PRESENT(hwrpb$il_vert_rez))
        return ENOTSUP;
#undef BOOTFB_FIELD_PRESENT
    /* Eight-byte x86 restart-block signature: "X86_RPB\0". */
    if (restart->hwrpb$iq_ident != UINT64_C(0x004250525f363858))
        return ENOTSUP;
    physical = restart->hwrpb$iq_framebuffer_pa;
    length = restart->hwrpb$iq_framebuffer_size;
    width = restart->hwrpb$il_horiz_rez;
    height = restart->hwrpb$il_vert_rez;
    if (!physical || !length || !width || !height) return ENODEV;
    if (length - 1u > UINT64_MAX - physical) return EOVERFLOW;
    info->physical_address = physical;
    info->byte_length = length;
    info->width = width;
    info->height = height;
    info->stride_bytes = 0;
    info->pixel_format = 0;
    return 0;
}

static int
vms_kernel_request(struct vms_request *request)
{
    vms_pcb32 process = CTL$GL_PCB;
    vms_adp32 adapter;
    unsigned int old_ipl = mfpr_ipl;
    unsigned int visited = 0;

    if (process == NULL || old_ipl > IPL$_ASTDEL) {
        request->error = EIO;
        return SS$_NORMAL;
    }
    if (request->operation == VMS_BOOTFB) {
        request->error = vms_kernel_bootfb(&request->bootfb);
        request->completed = 1;
        return SS$_NORMAL;
    }
    (void)setipl(IPL$_ASTDEL);
    (void)sch_std$iolockr(process);
    if (!request->inventory)
        request->error = ENODEV;

    for (adapter = IOC$GL_ADPLIST; adapter; adapter = adapter->adp$l_link) {
        BUSARRAY_HEADER *array;
        BUSARRAYENTRY *entries;
        unsigned int count;
        unsigned int index;

        if (++visited > VMS_MAX_ADAPTERS) {
            request->error = EOVERFLOW;
            break;
        }
        if (adapter->adp$w_size < offsetof(ADP, adp$ps_bus_array) +
                                   sizeof(adapter->adp$ps_bus_array)) {
            request->error = EIO;
            break;
        }
        array = adapter->adp$ps_bus_array;
        if (array == NULL || (array->busarray$l_bus_type != BUS$_PCI
#ifdef BUS$_PCIE
                             && array->busarray$l_bus_type != BUS$_PCIE
#endif
                            ))
            continue;
        if (array->busarray$ps_parent_adp != adapter) {
            request->error = EIO;
            break;
        }
        count = array->busarray$l_bus_node_cnt;
        if (count > VMS_MAX_BUS_ENTRIES) {
            request->error = EOVERFLOW;
            break;
        }
        if (!request->inventory && adapter->adp$l_tr != request->target.tr)
            continue;
        entries = (BUSARRAYENTRY *)array->busarray$q_entry_list;
        for (index = 0; index < count; index++) {
            unsigned int vendor;
            int error;

            if (!request->inventory) {
                if (entries[index].busarray$q_node_number ==
                    request->target.node) {
                    request->error = vms_read_entry(adapter, request);
                    goto unlock;
                }
                continue;
            }
            vendor = (unsigned int)entries[index].busarray$q_hw_id & 0xffffu;
            if (vendor == 0 || vendor == 0xffffu)
                continue;
            if (request->count == VMS_MAX_DEVICES) {
                request->error = EOVERFLOW;
                goto unlock;
            }
            error = vms_observe(adapter, &entries[index],
                                &request->devices[request->count]);
            if (error == ENODEV)
                continue;
            if (error) {
                request->error = error;
                goto unlock;
            }
            request->count++;
        }
    }

unlock:
    sch_std$iounlock(process);
    (void)setipl(old_ipl);
    return SS$_NORMAL;
}

static int
vms_request(struct vms_request *request)
{
    unsigned __int64 arguments[2];
    unsigned int status;

    arguments[0] = 1;
    arguments[1] = (uintptr_t)request;
    status = sys$cmkrnl_64(vms_kernel_request, arguments);
    return (status & 1) ? request->error : vms_status_error(status);
}

int
openvms_bootfb_query(struct openvms_bootfb *info)
{
    struct vms_request request;
    int error;

    if (!info) return EINVAL;
    memset(info, 0, sizeof(*info));
    /* Initialization holds this image resident for all kernel requests. */
    if (!pci_sys) return ENODEV;
    memset(&request, 0, sizeof(request));
    request.operation = VMS_BOOTFB;
    request.error = EIO;
    error = vms_request(&request);
    if (!error && !request.completed) error = EIO;
    if (!error) *info = request.bootfb;
    return error;
}

struct vms_config_context {
    vms_adp32 adapter;
    uint64_t node;
};

static int
vms_video_read(void *opaque, unsigned int offset, uint32_t *value)
{
    struct vms_config_context *context = opaque;
    return vms_config_long(context->adapter, context->node, offset, value);
}

static int
vms_video_write(void *opaque, unsigned int offset, unsigned int width,
                uint32_t value)
{
    struct vms_config_context *context = opaque;
    PCI_NODE_NUMBER node;
    unsigned int status;
    node.pci$q_pci_node_number_q = context->node;
    if (node.pci$v_pci_node_number_segment ||
        node.pci$v_pci_node_number_offset ||
        node.pci$v_pci_node_number_reserved)
        return ENOTSUP;
    /* IOC$K_WORD is right-justified, unlike historical WORD_LANED.
     * Never perform a dword command/status write: status contains W1C bits. */
    status = ioc$write_pci_config(context->adapter,
                node.pci$l_pci_node_number, offset,
                width == 2 ? IOC$K_WORD : IOC$K_LONGWORD, (int)value);
    return vms_status_error(status);
}

static int
vms_video_request(vms_adp32 adapter, struct vms_request *request)
{
    struct vms_config_context context;
    struct vms_bar_state *state = request->video;
    uint32_t value = 0, observed;
    unsigned int index;
    int error;
    context.adapter = adapter;
    context.node = request->target.node;
    if (!state || state->poisoned) return EIO;
    if (request->operation == VMS_PROBE)
        return vms_probe_bars(state, &context, vms_video_read, vms_video_write);
    if (!state->ready || request->operation != VMS_WRITE) return EACCES;
    /* Public config writes are deliberately restricted to command enable
     * bits or exact restoration of the original BARs/disabled ROM. */
    memcpy(&value, request->data, request->size);
    if (request->offset == 4 || request->offset == 5) {
        error = vms_video_read(&context, 4, &observed);
        if (error) return error;
        if (request->size == 1)
            value = request->offset == 4 ?
                ((observed & 0xff00u) | value) :
                ((observed & 0xffu) | (value << 8));
        value &= 0xffffu;
        if ((value & ~7u) != (state->command & ~7u)) return EACCES;
        error = vms_video_write(&context, 4, 2, value);
        if (!error) error = vms_video_read(&context, 4, &observed);
        if (!error && (uint16_t)observed != value) error = EIO;
    } else {
        index = request->offset == 0x30 ? 6 : (request->offset - 0x10) / 4;
        if (value != state->saved[index]) return EACCES;
        error = vms_video_write(&context, request->offset, 4, value);
        if (!error) error = vms_video_read(&context, request->offset, &observed);
        if (!error && observed != value) error = EIO;
    }
    if (!error) request->completed = request->size;
    return error;
}

static int
vms_owned(struct pci_device *device)
{
    struct vms_system *system = (struct vms_system *)pci_sys;
    return system && device && device == system->selected &&
           system->lock.id && system->video.ready && !system->video.poisoned;
}

static int
vms_selected_request(struct pci_device *device, struct vms_request *request)
{
    struct vms_system *system = (struct vms_system *)pci_sys;
    size_t index;
    if (!system || !device || device != system->selected) return ENODEV;
    for (index = 0; index < system->base.num_devices; index++)
        if (device == &system->base.devices[index].base) break;
    if (index == system->base.num_devices) return ENODEV;
    request->target = system->native[index];
    request->video = &system->video;
    return vms_request(request);
}

static int
vms_display_lease(struct vms_system *system)
{
    static $DESCRIPTOR(lock_name, "XORG$VMWARE$DISPLAY");
    unsigned int status;
    if (system->lock.id) return 0;
    status = sys$enqw(0, LCK$K_EXMODE, (struct _lksb *)&system->lock,
                LCK$M_NOQUEUE | LCK$M_SYSTEM, &lock_name, 0, 0, 0, 0, 0, 0);
    if (!(status & 1u) || !(system->lock.status & 1u) || !system->lock.id) {
        if (system->lock.id && !(sys$deq(system->lock.id, 0, 0, 0) & 1u))
            abort();
        system->lock.id = 0;
        return (status & 1u) ? EBUSY : vms_status_error(status);
    }
    return 0;
}

static int
vms_probe(struct pci_device *device)
{
    struct vms_system *system = (struct vms_system *)pci_sys;
    struct vms_request request;
    struct pci_device_private *private = (struct pci_device_private *)device;
    unsigned int i;
    int error;
    if (!system || device != system->selected) return ENOTSUP;
    if (system->video.poisoned) return EIO;
    if (system->video.ready) return 0;
    /* This backend is intentionally only the legacy VMware SVGA-II path.
     * No probing writes to other devices, bridges, or modern SVGA variants. */
    if (device->vendor_id != 0x15adu || device->device_id != 0x0405u ||
        (device->device_class & 0xffff00u) != 0x030000u ||
        (private->header_type & 0x7fu)) return ENOTSUP;
    error = vms_display_lease(system);
    if (error) return error;
    memset(&request, 0, sizeof(request));
    request.operation = VMS_PROBE;
    error = vms_selected_request(device, &request);
    if (error) return error;
    for (i = 0; i < 6; i++) {
        device->regions[i].base_addr = system->video.base[i];
        device->regions[i].bus_addr = system->video.base[i];
        device->regions[i].size = system->video.size[i];
        device->regions[i].is_IO = !!(system->video.saved[i] & 1u);
        device->regions[i].is_prefetchable = !device->regions[i].is_IO &&
                                             !!(system->video.saved[i] & 8u);
    }
    /* ROM reads/execution are not part of display-only support. */
    device->rom_size = 0;
    private->rom_base = 0;
    return 0;
}

static void
vms_destroy(void)
{
    struct vms_system *system = (struct vms_system *)pci_sys;

    if (system->maps) abort(); /* Never release ownership with live mappings. */
    if (system->lock.id) {
        unsigned int status = sys$deq(system->lock.id, 0, 0, 0);
        if (!(status & 1u)) abort();
    }
    free(system->native);
    if (system->unlock_image)
        (void)lib$unlock_image(0);
}

static int
vms_read_rom(struct pci_device *device, void *buffer)
{
    (void)device;
    (void)buffer;
    return ENOTSUP;
}

static int
vms_read(struct pci_device *device, void *data, pciaddr_t offset,
          pciaddr_t size, pciaddr_t *bytes_read)
{
    struct vms_system *system = (struct vms_system *)pci_sys;
    struct vms_request request;
    size_t index;
    int error;

    *bytes_read = 0;
    if (!vms_config_range_valid(offset, size))
        return EINVAL;
    if (size == 0)
        return 0;
    if (data == NULL)
        return EFAULT;
    for (index = 0; index < system->base.num_devices; index++)
        if (device == &system->base.devices[index].base)
            break;
    if (index == system->base.num_devices)
        return ENODEV;
    memset(&request, 0, sizeof(request));
    request.target = system->native[index];
    request.offset = (unsigned int)offset;
    request.size = (unsigned int)size;
    request.data = data;
    error = vms_request(&request);
    *bytes_read = request.completed;
    return error;
}

static int
vms_write(struct pci_device *device, const void *data, pciaddr_t offset,
           pciaddr_t size, pciaddr_t *bytes_written)
{
    struct vms_request request;
    int error;
    *bytes_written = 0;
    if (!vms_owned(device)) return EACCES;
    if (!data || (size != 1 && size != 2 && size != 4) ||
        offset > 256 - size || (offset & (size - 1))) return EINVAL;
    if ((size == 1 && offset != 4 && offset != 5) ||
        (size == 2 && offset != 4) ||
        (size == 4 && offset != 4 && offset != 0x30 &&
         (offset < 0x10 || offset > 0x24))) return ENOTSUP;
    memset(&request, 0, sizeof(request));
    request.operation = VMS_WRITE;
    request.offset = (unsigned int)offset;
    request.size = (unsigned int)size;
    request.data = (unsigned char *)data;
    error = vms_selected_request(device, &request);
    *bytes_written = request.completed;
    return error;
}

/* Both callers establish their own authority and allowed physical interval.
 * Keep one PFN mapper, mapping registry and deletion path for this backend.
 */
static int
vms_map_physical(struct vms_system *system, uint64_t base, uint64_t size,
                 unsigned int flags, void **address,
                 uint64_t allowed_base, uint64_t allowed_size)
{
    struct vms_mapping *map, *scan;
    uint64_t page_base, offset, length;
    unsigned int count = 0, status, section_flags, page_size = 0;
    int item = SYI$_PAGE_SIZE;
    if (!size || size - 1u > UINT64_MAX - base) return EOVERFLOW;
    status = lib$getsyi(&item, &page_size);
    if (!(status & 1u)) return vms_status_error(status);
    /* OpenVMS x86 uses 4 KiB physical frames and 8 KiB logical pages.
     * Reject changed OS geometry rather than guessing service units. */
    if (page_size != 8192u) return ENOTSUP;
    offset = base & (page_size - 1u);
    page_base = base - offset;
    if (size > UINT64_MAX - offset - (page_size - 1u)) return EOVERFLOW;
    length = (size + offset + page_size - 1u) & ~((uint64_t)page_size - 1u);
    if (!vms_range_contains(page_base, length, allowed_base, allowed_size))
        return EACCES;
    if (length > SIZE_MAX || length / page_size > UINT32_MAX ||
        page_base / 4096u > UINT32_MAX ||
        length - 1u > UINT64_MAX - page_base ||
        (page_base + length - 1u) / 4096u > UINT32_MAX)
        return EOVERFLOW;
    for (scan = system->maps; scan; scan = scan->next) count++;
    if (count >= 16) return ENOMEM;
    map = calloc(1, sizeof(*map));
    if (!map) return ENOMEM;
    {
        GENERIC_64 region = {VA$C_P2};
        map->region = region;
    }
    /* WRITE_COMBINE is only a hint; never substitute ordinary cached MMIO. */
    section_flags = SEC$M_EXPREG | SEC$M_NO_OVERMAP | SEC$M_PFNMAP |
                    SEC$M_UNCACHED | SEC$M_ARGS64;
    if (flags & PCI_DEV_MAP_FLAG_WRITABLE) section_flags |= SEC$M_WRT;
    status = sys$crmpsc_pfn_64(&map->region, (unsigned int)(page_base / 4096u),
                (unsigned int)(length / page_size), PSL$C_USER, section_flags,
                &map->base, &map->length);
    if (!(status & 1u) || !map->base || map->length != length) {
        /* Unexpected partial service results belong to process rundown;
         * do not release the hardware lease or invent a deletion extent. */
        if (map->base || map->length) abort();
        free(map);
        return (status & 1u) ? EIO : vms_status_error(status);
    }
    map->address = (void *)((uintptr_t)map->base + offset);
    map->requested = size;
    map->next = system->maps;
    system->maps = map;
    *address = map->address;
    return 0;
}

static int
vms_unmap_physical(struct vms_system *system, void *address, uint64_t size)
{
    struct vms_mapping **link, *map;
    void *deleted = NULL;
    unsigned __int64 length = 0;
    unsigned int status;
    for (link = &system->maps; *link; link = &(*link)->next)
        if ((*link)->address == address && (*link)->requested == size) break;
    if (!*link) return EINVAL;
    map = *link;
    status = sys$deltva_64(&map->region, map->base, map->length,
                           PSL$C_USER, &deleted, &length);
    if (!(status & 1u)) return vms_status_error(status);
    if (deleted != map->base || length != map->length) abort();
    *link = map->next;
    free(map);
    return 0;
}

static int
vms_map(struct pci_device *device, pciaddr_t base, pciaddr_t size,
         unsigned int flags, void **address, int legacy)
{
    struct vms_system *system = (struct vms_system *)pci_sys;
    uint64_t allowed_base = 0, allowed_size = 0;
    unsigned int i;
    if (!address) return EINVAL;
    *address = NULL;
    if (!vms_owned(device)) return EACCES;
    if (!size || base > UINT32_MAX || size > UINT32_MAX ||
        size > UINT64_C(0x100000000) - base) return EINVAL;
    if (flags & ~(PCI_DEV_MAP_FLAG_WRITABLE | PCI_DEV_MAP_FLAG_WRITE_COMBINE))
        return ENOTSUP;
    if (legacy) {
        if (!vms_range_contains(base, size, 0xa0000u, 0x20000u) ||
            (flags & PCI_DEV_MAP_FLAG_WRITE_COMBINE)) return EACCES;
        allowed_base = 0xa0000u;
        allowed_size = 0x20000u;
    } else {
        for (i = 0; i < 6; i++) {
            if (!(system->video.saved[i] & 1u) &&
                vms_range_contains(base, size, system->video.base[i],
                                     system->video.size[i])) {
                if ((flags & PCI_DEV_MAP_FLAG_WRITE_COMBINE) &&
                    !(system->video.saved[i] & 8u)) return EACCES;
                allowed_base = system->video.base[i];
                allowed_size = system->video.size[i];
                break;
            }
        }
        if (!allowed_size) return EACCES;
    }
    return vms_map_physical(system, base, size, flags, address,
                             allowed_base, allowed_size);
}

static int
vms_unmap(struct pci_device *device, void *address, pciaddr_t size)
{
    struct vms_system *system = (struct vms_system *)pci_sys;
    if (!system || !device || device != system->selected || !system->lock.id)
        return EACCES;
    return vms_unmap_physical(system, address, size);
}

int
openvms_bootfb_map(const struct openvms_bootfb *info, void **address)
{
    struct vms_system *system = (struct vms_system *)pci_sys;
    struct openvms_bootfb current;
    const char *exclusive = getenv("XORG_OPENVMS_FB_EXCLUSIVE");
    uint64_t used, offset, allowed_base, allowed_size;
    int error;

    if (!address) return EINVAL;
    *address = NULL;
    if (!system || system->selected || !exclusive || strcmp(exclusive, "1"))
        return EACCES;
    if (!info || !info->physical_address || !info->byte_length ||
        !info->width || !info->height || info->width > 32767u ||
        info->height > 32767u || (info->stride_bytes & 3u) ||
        (uint64_t)info->stride_bytes < (uint64_t)info->width * 4u ||
        (info->pixel_format != OPENVMS_BOOTFB_XRGB8888 &&
         info->pixel_format != OPENVMS_BOOTFB_XBGR8888))
        return EINVAL;
    if (info->byte_length - 1u > UINT64_MAX - info->physical_address)
        return EOVERFLOW;
    /* uint32_t pitch and height multiply without overflow in uint64_t. */
    used = (uint64_t)info->stride_bytes * info->height;
    if (used > info->byte_length) return EINVAL;
    error = vms_display_lease(system);
    if (error) return error;
    error = openvms_bootfb_query(&current);
    if (error) return error;
    if (info->physical_address != current.physical_address ||
        info->byte_length != current.byte_length ||
        info->width != current.width || info->height != current.height)
        return EAGAIN;
    /* PFN mappings necessarily include boundary pages. Authorize only the
     * pages containing the verified pitched image; return its pixel address.
     * The caller must never access the surrounding page padding.
     */
    offset = current.physical_address & 8191u;
    allowed_base = current.physical_address - offset;
    if (used > UINT64_MAX - offset - 8191u) return EOVERFLOW;
    allowed_size = (used + offset + 8191u) & ~UINT64_C(8191);
    return vms_map_physical(system, current.physical_address, used,
                             PCI_DEV_MAP_FLAG_WRITABLE, address,
                             allowed_base, allowed_size);
}

int
openvms_bootfb_unmap(void *address, uint64_t requested_bytes)
{
    struct vms_system *system = (struct vms_system *)pci_sys;
    if (!system || system->selected || !system->lock.id) return EACCES;
    return vms_unmap_physical(system, address, requested_bytes);
}

static int
vms_map_range(struct pci_device *device, struct pci_device_mapping *map)
{
    return vms_map(device, map->base, map->size, map->flags, &map->memory, 0);
}
static int
vms_unmap_range(struct pci_device *device, struct pci_device_mapping *map)
{
    return vms_unmap(device, map->memory, map->size);
}
static int
vms_map_legacy(struct pci_device *device, pciaddr_t base, pciaddr_t size,
                unsigned int flags, void **address)
{
    return vms_map(device, base, size, flags, address, 1);
}

static struct pci_io_handle *
vms_open_io(struct pci_io_handle *handle, struct pci_device *device,
             int bar, pciaddr_t base, pciaddr_t size)
{
    struct vms_system *system = (struct vms_system *)pci_sys;
    unsigned int i;
    if (!vms_owned(device)) { errno = EACCES; return NULL; }
    if (!size || base >= 65536u || size > 65536u - base ||
        (bar >= 0 && (bar >= 6 || !(system->video.saved[bar] & 1u) ||
            !vms_range_contains(base, size, system->video.base[bar],
                                 system->video.size[bar])))) {
        errno = EINVAL;
        return NULL;
    }
    for (i = 0; i < 16 && system->io[i]; i++) { }
    if (i == 16) { errno = EMFILE; return NULL; }
    memset(handle, 0, sizeof(*handle));
    handle->base = base;
    handle->size = size;
    handle->fd = -1;
    handle->is_legacy = bar < 0;
    system->io[i] = handle;
    return handle;
}
static struct pci_io_handle *
vms_open_legacy_io(struct pci_io_handle *handle, struct pci_device *device,
                    pciaddr_t base, pciaddr_t size)
{
    /* A namespace handle is not permission to use all ports. Every legacy
     * access is further restricted to the VGA ports and byte delay port. */
    return vms_open_io(handle, device, -1, base, size);
}
static void
vms_close_io(struct pci_device *device, struct pci_io_handle *handle)
{
    struct vms_system *system = (struct vms_system *)pci_sys;
    unsigned int i;
    if (!system || device != system->selected) abort();
    for (i = 0; i < 16 && system->io[i] != handle; i++) { }
    if (i == 16) abort();
    system->io[i] = NULL;
}

struct vms_port_request {
    unsigned int operation;
    unsigned short port;
    uint32_t value;
};
static int
vms_kernel_port(struct vms_port_request *request)
{
    request->value = pci_openvms_port_io(request->operation, request->port,
                                        request->value);
    return SS$_NORMAL;
}
static uint32_t
vms_port(struct pci_io_handle *handle, uint32_t reg, unsigned int operation,
          uint32_t value)
{
    struct vms_system *system = (struct vms_system *)pci_sys;
    struct vms_port_request request;
    unsigned __int64 arguments[2];
    unsigned int i, width, ast, status, restored;
    uint64_t port;
    if (!system || !vms_owned(system->selected)) goto denied;
    for (i = 0; i < 16 && system->io[i] != handle; i++) { }
    if (i == 16) goto denied;
    width = operation == VMS_IN8 || operation == VMS_OUT8 ? 1 :
            operation == VMS_IN16 || operation == VMS_OUT16 ? 2 : 4;
    if (!vms_range_contains(reg, width, 0, handle->size)) goto denied;
    port = handle->base + reg;
    if (port >= 65536u || width > 65536u - port) goto denied;
    if (handle->is_legacy &&
        !vms_range_contains(port, width, 0x3b0u, 0x0cu) &&
        !vms_range_contains(port, width, 0x3c0u, 0x20u) &&
        !(port == 0x80u && width == 1)) goto denied;
    ast = sys$setast(0);
    if (ast != SS$_WASSET && ast != SS$_WASCLR) {
        errno = vms_status_error(ast);
        return UINT32_MAX;
    }
    request.operation = operation;
    request.port = (unsigned short)port;
    request.value = value;
    arguments[0] = 1;
    arguments[1] = (uintptr_t)&request;
    status = sys$cmkrnl_64(vms_kernel_port, arguments);
    if (ast == SS$_WASSET) {
        restored = sys$setast(1);
        if (restored != SS$_WASCLR) abort();
    }
    if (!(status & 1u)) {
        errno = vms_status_error(status);
        return UINT32_MAX;
    }
    return request.value;
denied:
    errno = EACCES;
    return UINT32_MAX;
}
static uint8_t vms_read8(struct pci_io_handle *h, uint32_t r)
{ return (uint8_t)vms_port(h, r, VMS_IN8, 0); }
static uint16_t vms_read16(struct pci_io_handle *h, uint32_t r)
{ return (uint16_t)vms_port(h, r, VMS_IN16, 0); }
static uint32_t vms_read32(struct pci_io_handle *h, uint32_t r)
{ return vms_port(h, r, VMS_IN32, 0); }
static void vms_write8(struct pci_io_handle *h, uint32_t r, uint8_t v)
{ (void)vms_port(h, r, VMS_OUT8, v); }
static void vms_write16(struct pci_io_handle *h, uint32_t r, uint16_t v)
{ (void)vms_port(h, r, VMS_OUT16, v); }
static void vms_write32(struct pci_io_handle *h, uint32_t r, uint32_t v)
{ (void)vms_port(h, r, VMS_OUT32, v); }

static void
vms_destroy_device(struct pci_device *device)
{
    struct vms_system *system = (struct vms_system *)pci_sys;
    struct pci_device_private *private = (struct pci_device_private *)device;
    unsigned int i;
    if (device != system->selected) return;
    while (private->num_mappings)
        if (pci_device_unmap_range(device, private->mappings[0].memory,
                                    private->mappings[0].size)) abort();
    while (system->maps)
        if (vms_unmap(device, system->maps->address,
                        system->maps->requested)) abort();
    for (i = 0; i < 16; i++)
        if (system->io[i]) pci_device_close_io(device, system->io[i]);
    system->video.ready = 0;
}

/* No ownership query is provided: the common has_kernel_driver zero fallback
 * must not be interpreted as permission to claim or reconfigure a device. */
static const struct pci_system_methods vms_methods = {
    .destroy = vms_destroy,
    .destroy_device = vms_destroy_device,
    .read_rom = vms_read_rom,
    .probe = vms_probe,
    .map_range = vms_map_range,
    .unmap_range = vms_unmap_range,
    .read = vms_read,
    .write = vms_write,
    .fill_capabilities = pci_fill_capabilities_generic,
    .open_device_io = vms_open_io,
    .open_legacy_io = vms_open_legacy_io,
    .close_io = vms_close_io,
    .read8 = vms_read8, .read16 = vms_read16, .read32 = vms_read32,
    .write8 = vms_write8, .write16 = vms_write16, .write32 = vms_write32,
    .map_legacy = vms_map_legacy,
    .unmap_legacy = vms_unmap
};

int
pci_system_vms_create(void)
{
    struct vms_system *system;
    struct vms_request request;
    unsigned int status;
    size_t index;
    int error;
    const char *selection = getenv("PCIACCESS_OPENVMS_DEVICE");
    const char *exclusive = getenv("PCIACCESS_OPENVMS_EXCLUSIVE");
    unsigned int domain = 0, bus = 0, slot = 0, function = 0;
    int end = 0;

    /* Selecting a device never happens implicitly, including on single-GPU
     * machines. Existing discovery clients need not opt into resource access. */
    if (selection && !*selection && exclusive && !strcmp(exclusive, "0")) {
        /* Explicitly shadow inherited device-selection logicals for clients
         * that need enumeration only. No standard PCI access is enabled. */
        selection = NULL;
        exclusive = NULL;
    }
    if (selection || exclusive) {
        if (!selection || !exclusive || strcmp(exclusive, "1") ||
            strchr(selection, '-') || strchr(selection, '+') ||
            sscanf(selection, "%x:%x:%x.%x%n", &domain, &bus, &slot,
                    &function, &end) != 4 || selection[end] || domain ||
            bus > 255 || slot > 31 || function > 7)
            return EINVAL;
    }

    system = calloc(1, sizeof(*system));
    if (system == NULL)
        return ENOMEM;
    system->base.vgaarb_fd = -1;
    system->native = calloc(VMS_MAX_DEVICES, sizeof(*system->native));
    system->base.devices = calloc(VMS_MAX_DEVICES,
                                  sizeof(*system->base.devices));
    if (system->native == NULL || system->base.devices == NULL) {
        error = ENOMEM;
        goto fail;
    }
    /* Keep this image resident for the whole initialized lifetime.  IOC may
     * raise IPL internally.  Do not unlock a lock owned by our caller. */
    status = lib$lock_image(0);
    if (!(status & 1)) {
        error = vms_status_error(status);
        goto fail;
    }
    system->unlock_image = status == SS$_WASCLR;
    memset(&request, 0, sizeof(request));
    request.inventory = 1;
    request.devices = system->native;
    error = vms_request(&request);
    if (error)
        goto fail;

    /* Interpret OS node numbers only through the installed SDK. */
    for (index = 0; index < request.count; index++) {
        struct vms_device *candidate = &system->native[index];
        PCI_NODE_NUMBER node;

        node.pci$q_pci_node_number_q = candidate->node;
        if (node.pci$v_pci_node_number_segment ||
            node.pci$v_pci_node_number_offset ||
            node.pci$v_pci_node_number_reserved) {
            error = ENOTSUP;
            goto fail;
        }
        candidate->domain = node.pci$v_pci_node_number_segment;
        candidate->bus = node.pci$v_pci_node_number_bus;
        candidate->dev = node.pci$v_pci_node_number_device;
        candidate->func = node.pci$v_pci_node_number_function;
    }
    error = vms_compact_devices(system->native, request.count,
                                &system->base.num_devices);
    if (error)
        goto fail;
    for (index = 0; index < system->base.num_devices; index++) {
        const struct vms_device *candidate = &system->native[index];
        struct pci_device_private *private = &system->base.devices[index];
        struct pci_device *device = &private->base;

        device->domain = candidate->domain;
        device->domain_16 = (uint16_t)device->domain;
        device->bus = candidate->bus;
        device->dev = candidate->dev;
        device->func = candidate->func;
        device->vendor_id = candidate->id & 0xffffu;
        device->device_id = candidate->id >> 16;
        device->subvendor_id = candidate->subsystem & 0xffffu;
        device->subdevice_id = candidate->subsystem >> 16;
        device->device_class = candidate->class_revision >> 8;
        device->revision = candidate->class_revision & 0xffu;
        device->irq = -1; /* No validated OS interrupt-routing query yet. */
        private->header_type = (candidate->header >> 16) & 0xffu;
        if (selection && device->domain == domain && device->bus == bus &&
            device->dev == slot && device->func == function)
            system->selected = device;
    }
    if (selection && !system->selected) {
        error = ENODEV;
        goto fail;
    }
    system->base.methods = &vms_methods;
    pci_sys = &system->base;
    return 0;

fail:
    if (system->unlock_image)
        (void)lib$unlock_image(0);
    free(system->base.devices);
    free(system->native);
    free(system);
    return error;
}
