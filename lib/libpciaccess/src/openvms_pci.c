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
 * Native OpenVMS x86-64 discovery and read-only configuration access.
 *
 * References: OpenVMS AXP Device Support Developer's Guide AA-Q28SA-TE, 
 * sections 13.5-13.13; Device Support Reference, AA-Q28PA-TE,
 * ADP/BUSARRAY and SCH_STD$IOLOCKR/IOUNLOCK; current VSI System Services,
 * RTL Library and System Analysis manuals.  
 *
 * The executive interfaces require CMKRNL privilege and LINK/SYSEXE.  
 * This is not a supported user-mode PCI service or a graphics resource backend.
 * No configuration writes, BAR-sizing writes, MMIO or port accesses occur.
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
#include <stdlib.h>
#include <string.h>
#include <adpdef.h>
#include <busarraydef.h>
#include <busdef.h>
#include <ioc_routines.h>
#include <ipldef.h>
#include <lib$routines.h>
#include <pcbdef.h>
#include <pcidef.h>
#include <sch_routines.h>
#include <ssdef.h>
#include <starlet.h>
#include <vms_macros.h>

#include "pciaccess.h"
#include "pciaccess_private.h"
#include "openvms_pci_private.h"

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
};

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

static void
vms_destroy(void)
{
    struct vms_system *system = (struct vms_system *)pci_sys;

    free(system->native);
    if (system->unlock_image)
        (void)lib$unlock_image(0);
}

static int
vms_unsupported_device(struct pci_device *device)
{
    (void)device;
    return ENOTSUP;
}

static int
vms_read_rom(struct pci_device *device, void *buffer)
{
    (void)device;
    (void)buffer;
    return ENOTSUP;
}

static int
vms_unsupported_mapping(struct pci_device *device,
                        struct pci_device_mapping *mapping)
{
    (void)device;
    (void)mapping;
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
    (void)device;
    (void)data;
    (void)offset;
    (void)size;
    *bytes_written = 0;
    return ENOTSUP;
}

/* No ownership query is provided: the common has_kernel_driver zero fallback
 * must not be interpreted as permission to claim or reconfigure a device. */
static const struct pci_system_methods vms_methods = {
    .destroy = vms_destroy,
    .read_rom = vms_read_rom,
    .probe = vms_unsupported_device,
    .map_range = vms_unsupported_mapping,
    .unmap_range = vms_unsupported_mapping,
    .read = vms_read,
    .write = vms_write,
    .fill_capabilities = pci_fill_capabilities_generic
};

int
pci_system_vms_create(void)
{
    struct vms_system *system;
    struct vms_request request;
    unsigned int status;
    size_t index;
    int error;

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
