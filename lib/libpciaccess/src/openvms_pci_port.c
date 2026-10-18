/* Copyright (c) 2026 Kisara Development LLC
 * SPDX-License-Identifier: MIT
 * x86 I/O instruction leaf, callable only from the kernel-mode callback.
 * Compile with clang -mno-red-zone: interrupts may use the kernel stack.
 */
#include "openvms_pci_port.h"
#if !defined(__clang__) || !defined(__x86_64)
#error OpenVMS x86-64 clang is required.
#endif
uint32_t
pci_openvms_port_io(unsigned int operation, unsigned short port, uint32_t value)
{
    uint8_t byte;
    uint16_t word;
    switch (operation) {
    case VMS_IN8:
        __asm__ __volatile__("inb %%dx, %%al" : "=a"(byte) : "d"(port) : "memory");
        return byte;
    case VMS_IN16:
        __asm__ __volatile__("inw %%dx, %%ax" : "=a"(word) : "d"(port) : "memory");
        return word;
    case VMS_IN32:
        __asm__ __volatile__("inl %%dx, %%eax" : "=a"(value) : "d"(port) : "memory");
        return value;
    case VMS_OUT8:
        __asm__ __volatile__("outb %%al, %%dx" : : "a"((uint8_t)value), "d"(port) : "memory");
        return value;
    case VMS_OUT16:
        __asm__ __volatile__("outw %%ax, %%dx" : : "a"((uint16_t)value), "d"(port) : "memory");
        return value;
    case VMS_OUT32:
        __asm__ __volatile__("outl %%eax, %%dx" : : "a"(value), "d"(port) : "memory");
        return value;
    }
    return UINT32_MAX;
}
