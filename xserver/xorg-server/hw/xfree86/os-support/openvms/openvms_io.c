/* Original OpenVMS implementation of the upstream NO_INLINE port-I/O ABI.
 * All access passes through the selected libpciaccess device's bounded native
 * backend. This layer neither maps physical memory nor enters kernel mode.
 */
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <pciaccess.h>
#include "xf86.h"
#include "compiler.h"

static struct pci_device *video;
static struct pci_io_handle *bar_io[6], *legacy;

static void
select_video(void)
{
    const char *selected = getenv("PCIACCESS_OPENVMS_DEVICE");
    unsigned int domain, bus, dev, func;
    int end = 0;
    struct pci_slot_match match;
    struct pci_device_iterator *iterator;
    if (video)
        return;
    if (!selected || sscanf(selected, "%x:%x:%x.%x%n", &domain, &bus, &dev,
                            &func, &end) != 4 || selected[end] ||
        domain > 0xffff || bus > 255 || dev > 31 || func > 7)
        FatalError("OpenVMS Xorg needs PCIACCESS_OPENVMS_DEVICE=domain:bus:dev.func (hex)\n");
    match.domain = domain;
    match.bus = bus;
    match.dev = dev;
    match.func = func;
    match.match_data = 0;
    iterator = pci_slot_match_iterator_create(&match);
    if (!iterator)
        FatalError("OpenVMS Xorg cannot find its selected PCI device\n");
    video = pci_device_next(iterator);
    pci_iterator_destroy(iterator);
    if (!video || pci_device_probe(video))
        FatalError("OpenVMS Xorg selected PCI device is not available for I/O\n");
}

static struct pci_io_handle *
port_handle(unsigned short port, unsigned int width, uint32_t *offset)
{
    unsigned int i;
    select_video();
    for (i = 0; i < 6; ++i) {
        const struct pci_mem_region *r = &video->regions[i];
        if (r->is_IO && r->size && port >= r->base_addr &&
            (uint64_t)port + width <= r->base_addr + r->size) {
            if (!bar_io[i])
                bar_io[i] = pci_device_open_io(video, r->base_addr, r->size);
            if (!bar_io[i])
                FatalError("OpenVMS Xorg cannot open its PCI I/O region\n");
            *offset = (uint32_t)(port - r->base_addr);
            return bar_io[i];
        }
    }
    /* libpciaccess independently checks the actual legacy VGA port ranges. */
    if (!legacy)
        legacy = pci_legacy_open_io(video, 0, 65536);
    if (!legacy)
        FatalError("OpenVMS Xorg cannot open its legacy VGA I/O range\n");
    *offset = port;
    return legacy;
}

void
OpenVMSClosePortIO(void)
{
    unsigned int i;
    if (video) {
        for (i = 0; i < 6; ++i) {
            if (bar_io[i])
                pci_device_close_io(video, bar_io[i]);
            bar_io[i] = NULL;
        }
        if (legacy)
            pci_device_close_io(video, legacy);
    }
    legacy = NULL;
    video = NULL;
}

static uint32_t
read_port(unsigned short p, unsigned int width)
{
    uint32_t offset, value;
    struct pci_io_handle *h = port_handle(p, width, &offset);
    errno = 0;
    value = width == 1 ? pci_io_read8(h, offset) :
            width == 2 ? pci_io_read16(h, offset) : pci_io_read32(h, offset);
    if (errno)
        FatalError("OpenVMS Xorg port read failed: %04x/%u: %d\n", p, width, errno);
    return value;
}

static void
write_port(unsigned short p, unsigned int width, uint32_t value)
{
    uint32_t offset;
    struct pci_io_handle *h = port_handle(p, width, &offset);
    errno = 0;
    if (width == 1) pci_io_write8(h, offset, (uint8_t)value);
    else if (width == 2) pci_io_write16(h, offset, (uint16_t)value);
    else pci_io_write32(h, offset, value);
    if (errno)
        FatalError("OpenVMS Xorg port write failed: %04x/%u: %d\n", p, width, errno);
}

unsigned int inb(unsigned short p) { return read_port(p, 1); }
unsigned int inw(unsigned short p) { return read_port(p, 2); }
unsigned int inl(unsigned short p) { return read_port(p, 4); }
void outb(unsigned short p, unsigned char v) { write_port(p, 1, v); }
void outw(unsigned short p, unsigned short v) { write_port(p, 2, v); }
void outl(unsigned short p, unsigned int v) { write_port(p, 4, v); }
