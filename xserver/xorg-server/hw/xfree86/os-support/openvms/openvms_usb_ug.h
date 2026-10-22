/* SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Kisara Development LLC
 */
#ifndef OPENVMS_USB_UG_H
#define OPENVMS_USB_UG_H

#include <stddef.h>

typedef struct openvms_usb_ug openvms_usb_ug;

/* Explicit UGA device only: no enumeration, driver loading or configuration
 * changes. protocol is OPENVMS_USB_HID_KEYBOARD or OPENVMS_USB_HID_MOUSE.
 */
int openvms_usb_ug_open(openvms_usb_ug **result, const char *device,
                        unsigned protocol, char *error, size_t error_capacity);

/* At most one outstanding request and one delivered report per invocation.
 * Returns 1 with a report, 0 while pending, or -1 on permanent failure.
 */
int openvms_usb_ug_poll(openvms_usb_ug *source, unsigned char *report,
                        size_t capacity, size_t *length);

const char *openvms_usb_ug_error(const openvms_usb_ug *source);

/* Sticky after any incomplete close, including cleanup inside a failed open.
 * The server must retain its input ownership lease until process rundown.
 */
int openvms_usb_ug_cleanup_pending(void);

/* Returns 1 after deassignment and freeing, 0 if request storage must remain
 * alive until process rundown, or device-state restoration failed. The handle
 * is invalid after either result.
 * A NULL handle is harmless and returns 1.
 */
int openvms_usb_ug_close(openvms_usb_ug *source);

#endif
