/* Copyright (c) 2026 Kisara Development LLC
 * SPDX-License-Identifier: MIT
 */
#ifndef OPENVMS_PCI_PORT_H
#define OPENVMS_PCI_PORT_H
#include <stdint.h>
enum { VMS_IN8, VMS_IN16, VMS_IN32, VMS_OUT8, VMS_OUT16, VMS_OUT32 };
uint32_t pci_openvms_port_io(unsigned int, unsigned short, uint32_t);
#endif
