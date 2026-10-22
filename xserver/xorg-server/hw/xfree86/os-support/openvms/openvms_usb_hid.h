/* SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Kisara Development LLC
 */
#ifndef OPENVMS_USB_HID_H
#define OPENVMS_USB_HID_H

#include <stddef.h>
#include <stdint.h>

#define OPENVMS_USB_HID_KEYBOARD 1U
#define OPENVMS_USB_HID_MOUSE 2U
#define OPENVMS_USB_HID_KEYBOARD_REPORT_SIZE 8U
#define OPENVMS_USB_HID_MOUSE_REPORT_SIZE 3U
#define OPENVMS_USB_HID_MAX_BOOT_REPORT_SIZE 8U

typedef enum {
    OPENVMS_USB_HID_OK = 0,
    OPENVMS_USB_HID_INVALID_ARGUMENT = -1,
    OPENVMS_USB_HID_INVALID_REPORT = -2,
    OPENVMS_USB_HID_KEYBOARD_ERROR = -3,
    OPENVMS_USB_HID_INVALID_DESCRIPTOR = -4,
    OPENVMS_USB_HID_NOT_FOUND = -5,
    OPENVMS_USB_HID_AMBIGUOUS = -6
} openvms_usb_hid_status;

typedef struct {
    uint8_t modifiers;
    uint8_t reserved;
    uint8_t keys[6];
} openvms_usb_hid_keyboard_state;

typedef void (*openvms_usb_hid_key_callback)(void *context, uint8_t usage,
                                          int pressed);
typedef void (*openvms_usb_hid_mouse_callback)(void *context, uint8_t buttons,
                                            int8_t dx, int8_t dy);

/* Initialize state to {0}; thereafter only these routines may change it.
 * Callbacks must not reenter this decoder or mutate its state.
 */
openvms_usb_hid_status openvms_usb_hid_keyboard_report(
    openvms_usb_hid_keyboard_state *state, const uint8_t *report, size_t length,
    openvms_usb_hid_key_callback callback, void *context);

openvms_usb_hid_status openvms_usb_hid_keyboard_release_all(
    openvms_usb_hid_keyboard_state *state,
    openvms_usb_hid_key_callback callback, void *context);

/* Decode the boot prefix of a 3..8-byte report after SET_PROTOCOL(0).
 * Only the low three button bits and signed X/Y bytes are delivered.
 * Extension bytes and the upper button-byte bits have no meaning here;
 * in particular, an extension is never interpreted as a wheel report.
 */
openvms_usb_hid_status openvms_usb_hid_mouse_report(
    const uint8_t *report, size_t length,
    openvms_usb_hid_mouse_callback callback, void *context);

typedef struct {
    uint8_t configuration_value;
    uint8_t interface_number;
    uint8_t alternate_setting;
    uint8_t protocol;
    uint8_t endpoint_address;
    uint8_t interval;
    uint16_t max_packet_size;
} openvms_usb_hid_boot_interface;

/* Validate one complete configuration (length must equal wTotalLength).
 * Select exactly one alternate-zero HID boot interface of the requested
 * protocol with one interrupt IN endpoint and a well-formed HID descriptor.
 */
openvms_usb_hid_status openvms_usb_hid_find_boot_interface(
    const uint8_t *configuration, size_t length, uint8_t protocol,
    openvms_usb_hid_boot_interface *selected);

#endif
