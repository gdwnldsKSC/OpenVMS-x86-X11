/* Copyright (c) 2026 Kisara Development LLC
 * SPDX-License-Identifier: MIT
 * Portable, bounded PCI type-0 BAR transaction; no executive definitions.
 */
#ifndef OPENVMS_PCI_RESOURCE_H
#define OPENVMS_PCI_RESOURCE_H
#include <errno.h>
#include <stdint.h>
#include <string.h>

struct vms_bar_state {
    uint32_t saved[7], mask[7], base[6], size[6];
    uint16_t command;
    int ready, poisoned;
};

typedef int (*vms_cfg_read_fn)(void *, unsigned int, uint32_t *);
typedef int (*vms_cfg_write_fn)(void *, unsigned int, unsigned int, uint32_t);

static unsigned int
vms_bar_offset(unsigned int index)
{
    return index == 6 ? 0x30 : 0x10 + 4 * index;
}

static int
vms_range_contains(uint64_t base, uint64_t size, uint64_t start, uint64_t span)
{
    return size && span && base >= start && base - start < span &&
           size <= span - (base - start);
}

static int
vms_decode_bars(struct vms_bar_state *state)
{
    unsigned int i, j;
    for (i = 0; i < 6; i++) {
        uint32_t flags = state->saved[i] & 1 ? 3u : 15u;
        uint32_t mask = state->mask[i] & ~flags;
        uint32_t size;
        state->base[i] = state->saved[i] & ~flags;
        state->size[i] = 0;
        if (!mask) {
            if (state->base[i]) return EIO;
            continue;
        }
        if ((state->mask[i] & flags) != (state->saved[i] & flags))
            return EIO;
        /* PCI permits an I/O BAR to implement only the low 16 address bits. */
        if ((state->saved[i] & 1u) && !(mask & 0xffff0000u))
            mask |= 0xffff0000u;
        size = ~mask + 1u;
        if (!size || (size & (size - 1u)) || !state->base[i] ||
            (state->base[i] & (size - 1u)))
            return EIO;
        if (state->saved[i] & 1u) {
            if ((uint64_t)state->base[i] + size > 0x10000u) return ENOTSUP;
        } else {
            if (state->base[i] < 0x100000u ||
                (uint64_t)state->base[i] + size > UINT64_C(0x100000000))
                return ENOTSUP;
        }
        for (j = 0; j < i; j++) {
            if (state->size[j] &&
                (state->saved[i] & 1u) == (state->saved[j] & 1u) &&
                (uint64_t)state->base[i] < (uint64_t)state->base[j] + state->size[j] &&
                (uint64_t)state->base[j] < (uint64_t)state->base[i] + size)
                return EIO;
        }
        state->size[i] = size;
    }
    return 0;
}

/* All configuration snapshots precede the first write. Once writes start,
 * every saved BAR is restored and checked on every exit. Decoding remains
 * disabled if restoration cannot be proved. The caller serializes this whole
 * transaction, not just individual accesses. No allocation or logging here.
 */
static int
vms_probe_bars(struct vms_bar_state *state, void *context,
               vms_cfg_read_fn read, vms_cfg_write_fn write)
{
    unsigned int i;
    uint32_t value;
    int error, restored = 1;

    memset(state, 0, sizeof(*state));
    error = read(context, 4, &value);
    if (error) return error;
    state->command = (uint16_t)value;
    for (i = 0; i < 7; i++) {
        error = read(context, vms_bar_offset(i), &state->saved[i]);
        if (error) return error;
        if (i < 6 && ((state->saved[i] & 1u) ?
                     (state->saved[i] & 2u) : (state->saved[i] & 6u)))
            return ENOTSUP;
    }
    if (state->saved[6] & 1u) return EBUSY;
    error = write(context, 4, 2, state->command & ~7u);
    if (!error) error = read(context, 4, &value);
    if (!error && (uint16_t)value != (state->command & ~7u)) error = EIO;
    for (i = 0; !error && i < 7; i++) {
        error = write(context, vms_bar_offset(i), 4,
                      i == 6 ? 0xfffff800u : 0xffffffffu);
        if (!error) error = read(context, vms_bar_offset(i), &state->mask[i]);
        if (!error) error = write(context, vms_bar_offset(i), 4, state->saved[i]);
    }
    /* Retrying restoration is intentional even after a successful probe. */
    for (i = 0; i < 7; i++) {
        int result = write(context, vms_bar_offset(i), 4, state->saved[i]);
        if (!result) result = read(context, vms_bar_offset(i), &value);
        if (result || value != state->saved[i]) restored = 0;
    }
    if (write(context, 4, 2, restored ? state->command : state->command & ~7u) ||
        read(context, 4, &value) ||
        (uint16_t)value != (restored ? state->command : state->command & ~7u))
        restored = 0;
    if (!restored) {
        (void)write(context, 4, 2, state->command & ~7u);
        state->poisoned = 1;
        return EIO;
    }
    if (!error) error = vms_decode_bars(state);
    state->ready = error == 0;
    return error;
}

#endif
