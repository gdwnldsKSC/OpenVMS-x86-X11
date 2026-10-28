/* Copyright (c) 2026 Kisara Development LLC
 * SPDX-License-Identifier: MIT
 * Native boot framebuffer extension to this port's libpciaccess backend.
 */
#ifndef OPENVMS_BOOTFB_H
#define OPENVMS_BOOTFB_H

#include <stdint.h>

enum openvms_bootfb_format {
    OPENVMS_BOOTFB_XRGB8888 = 1, /* Little-endian bytes B, G, R, X. */
    OPENVMS_BOOTFB_XBGR8888 = 2  /* Little-endian bytes R, G, B, X. */
};

struct openvms_bootfb {
    uint64_t physical_address;
    uint64_t byte_length;
    uint32_t width;
    uint32_t height;
    uint32_t stride_bytes;
    uint32_t pixel_format;
};

/* All calls return zero or an errno value and require pci_system_init().
 * Query clears *info on failure. Its successful result leaves stride/format
 * zero because the verified HWRPB fields do not supply them.
 */
int openvms_bootfb_query(struct openvms_bootfb *info);

/* Map the same boot descriptor with explicitly verified stride/format.
 * Requires no selected PCI device and XORG_OPENVMS_FB_EXCLUSIVE=1: the
 * operator asserts serial-console ownership and no competing video driver.
 * The existing display lease serializes cooperating Xorg instances only.
 * The returned address exposes stride_bytes * height bytes; physical address
 * and allocation bounds are rechecked against the OS under that lease.
 * Calls must be serialized in ordinary process context, outside ASTs.
 */
int openvms_bootfb_map(const struct openvms_bootfb *info, void **address);

/* requested_bytes is the original stride_bytes * height. A failed unmap
 * leaves the mapping owned. Unmap every mapping before pci_system_cleanup();
 * the common display lease remains held until that normal cleanup.
 */
int openvms_bootfb_unmap(void *address, uint64_t requested_bytes);

#endif
