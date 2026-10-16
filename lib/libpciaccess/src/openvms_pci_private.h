/* Copyright (c) 2026 Kisara Development LLC
 * SPDX-License-Identifier: MIT
 * Portable metadata helpers; no executive structures or OS declarations.
 */
#ifndef OPENVMS_PCI_PRIVATE_H
#define OPENVMS_PCI_PRIVATE_H

#include <errno.h>
#include <stddef.h>
#include <stdint.h>

struct vms_device {
    uint64_t node;
    uint32_t domain;
    uint32_t tr;
    uint32_t id;
    uint32_t class_revision;
    uint32_t header;
    uint32_t subsystem;
    uint8_t bus;
    uint8_t dev;
    uint8_t func;
    int inventory_matches;
};

/* Compact aliases in place, keeping first-seen enumeration order.  Neither
 * TR nor a stale inventory HWID identifies a PCI function. */
static int
vms_compact_devices(struct vms_device *devices, size_t count, size_t *unique)
{
    size_t input;

    *unique = 0;
    for (input = 0; input < count; input++) {
        struct vms_device candidate = devices[input];
        size_t output;

        for (output = 0; output < *unique; output++) {
            const struct vms_device *old = &devices[output];
            if (candidate.domain == old->domain && candidate.bus == old->bus &&
                candidate.dev == old->dev && candidate.func == old->func)
                break;
        }
        if (output < *unique) {
            struct vms_device *old = &devices[output];
            if (candidate.id != old->id ||
                candidate.class_revision != old->class_revision ||
                candidate.header != old->header ||
                candidate.subsystem != old->subsystem)
                return EIO;
            if (candidate.inventory_matches > old->inventory_matches ||
                (candidate.inventory_matches == old->inventory_matches &&
                 candidate.tr < old->tr))
                *old = candidate;
        } else {
            devices[(*unique)++] = candidate;
        }
    }
    return 0;
}

static int
vms_config_range_valid(uint64_t offset, uint64_t size)
{
    return offset <= 256 && size <= 256 - offset;
}

#endif
