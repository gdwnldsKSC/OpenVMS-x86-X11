/* SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Kisara Development LLC
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * USB HID 1.11, sections 4.2/4.3/6.2.1/7.2.6, appendices B/C/E:
 * https://www.usb.org/sites/default/files/hid1_11.pdf
 * USB standard descriptor field documentation:
 * https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/usbspec/ns-usbspec-_usb_configuration_descriptor
 * https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/usbspec/ns-usbspec-_usb_interface_descriptor
 * https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/usbspec/ns-usbspec-_usb_endpoint_descriptor
 */
#include "openvms_usb_hid.h"

static int
has_key(const uint8_t keys[6], uint8_t usage)
{
    unsigned int i;

    for (i = 0; i < 6; ++i)
        if (keys[i] == usage)
            return 1;
    return 0;
}

static void
keyboard_commit(openvms_usb_hid_keyboard_state *state,
                const openvms_usb_hid_keyboard_state *next,
                openvms_usb_hid_key_callback callback, void *context)
{
    openvms_usb_hid_keyboard_state previous = *state;
    unsigned int i;

    *state = *next;
    for (i = 0; i < 6; ++i)
        if (previous.keys[i] != 0 && !has_key(next->keys, previous.keys[i]))
            callback(context, previous.keys[i], 0);
    for (i = 0; i < 8; ++i)
        if ((previous.modifiers & (1U << i)) &&
            !(next->modifiers & (1U << i)))
            callback(context, (uint8_t) (0xe0U + i), 0);
    for (i = 0; i < 8; ++i)
        if (!(previous.modifiers & (1U << i)) &&
            (next->modifiers & (1U << i)))
            callback(context, (uint8_t) (0xe0U + i), 1);
    for (i = 0; i < 6; ++i)
        if (next->keys[i] != 0 && !has_key(previous.keys, next->keys[i]))
            callback(context, next->keys[i], 1);
}

openvms_usb_hid_status
openvms_usb_hid_keyboard_report(openvms_usb_hid_keyboard_state *state,
                               const uint8_t *report, size_t length,
                               openvms_usb_hid_key_callback callback,
                               void *context)
{
    openvms_usb_hid_keyboard_state next = {0};
    unsigned int i, count = 0;

    if (state == NULL || report == NULL || callback == NULL)
        return OPENVMS_USB_HID_INVALID_ARGUMENT;
    if (length != OPENVMS_USB_HID_KEYBOARD_REPORT_SIZE || report[1] != 0)
        return OPENVMS_USB_HID_INVALID_REPORT;

    next.modifiers = report[0];
    for (i = 2; i < OPENVMS_USB_HID_KEYBOARD_REPORT_SIZE; ++i) {
        uint8_t usage = report[i];
        unsigned int at;

        if (usage >= 1 && usage <= 3)
            return OPENVMS_USB_HID_KEYBOARD_ERROR;
        if (usage >= 0xe0)
            return OPENVMS_USB_HID_INVALID_REPORT;
        if (usage == 0 || has_key(next.keys, usage))
            continue;
        at = count;
        while (at > 0 && next.keys[at - 1] > usage) {
            next.keys[at] = next.keys[at - 1];
            --at;
        }
        next.keys[at] = usage;
        ++count;
    }
    keyboard_commit(state, &next, callback, context);
    return OPENVMS_USB_HID_OK;
}

openvms_usb_hid_status
openvms_usb_hid_keyboard_release_all(openvms_usb_hid_keyboard_state *state,
                                    openvms_usb_hid_key_callback callback,
                                    void *context)
{
    const openvms_usb_hid_keyboard_state empty = {0};

    if (state == NULL || callback == NULL)
        return OPENVMS_USB_HID_INVALID_ARGUMENT;
    keyboard_commit(state, &empty, callback, context);
    return OPENVMS_USB_HID_OK;
}

static int8_t
signed_byte(uint8_t value)
{
    /* Do not depend on an implementation-defined unsigned-to-signed cast. */
    int result = value;

    if (result >= 128)
        result -= 256;
    return (int8_t) result;
}

openvms_usb_hid_status
openvms_usb_hid_mouse_report(const uint8_t *report, size_t length,
                            openvms_usb_hid_mouse_callback callback,
                            void *context)
{
    if (report == NULL || callback == NULL)
        return OPENVMS_USB_HID_INVALID_ARGUMENT;
    if (length < OPENVMS_USB_HID_MOUSE_REPORT_SIZE ||
        length > OPENVMS_USB_HID_MAX_BOOT_REPORT_SIZE)
        return OPENVMS_USB_HID_INVALID_REPORT;
    callback(context, (uint8_t) (report[0] & 7U), signed_byte(report[1]),
             signed_byte(report[2]));
    return OPENVMS_USB_HID_OK;
}

static uint16_t
little_endian_word(const uint8_t *bytes)
{
    return (uint16_t) ((uint16_t) bytes[0] | ((uint16_t) bytes[1] << 8));
}

static int
mark_bit(uint8_t bits[32], uint8_t value)
{
    uint8_t mask = (uint8_t) (1U << (value & 7U));
    unsigned int index = value >> 3;

    if (bits[index] & mask)
        return 0;
    bits[index] |= mask;
    return 1;
}

static int
valid_hid_descriptor(const uint8_t *descriptor, size_t length)
{
    unsigned int i, report_count = 0, physical_count = 0;

    if (length < 9 || descriptor[5] == 0 ||
        length != 6U + 3U * descriptor[5])
        return 0;
    for (i = 6; i < length; i += 3) {
        if (little_endian_word(descriptor + i + 1) == 0)
            return 0;
        if (descriptor[i] == 0x22)
            ++report_count;
        else if (descriptor[i] == 0x23)
            ++physical_count;
        else
            return 0;
    }
    return report_count == 1 && physical_count <= 1;
}

typedef struct {
    openvms_usb_hid_boot_interface value;
    uint32_t endpoints;
    unsigned int expected_endpoints;
    unsigned int endpoint_count;
    unsigned int input_count;
    unsigned int hid_count;
    int present;
    int candidate;
    int is_hid;
} interface_scan;

static openvms_usb_hid_status
finish_interface(const interface_scan *scan,
                 openvms_usb_hid_boot_interface *match,
                 unsigned int *match_count)
{
    if (!scan->present)
        return OPENVMS_USB_HID_OK;
    if (scan->endpoint_count != scan->expected_endpoints)
        return OPENVMS_USB_HID_INVALID_DESCRIPTOR;
    if (!scan->candidate)
        return OPENVMS_USB_HID_OK;
    if (scan->hid_count != 1 || scan->input_count == 0)
        return OPENVMS_USB_HID_INVALID_DESCRIPTOR;
    if (scan->input_count > 1)
        return OPENVMS_USB_HID_AMBIGUOUS;
    *match = scan->value;
    ++*match_count;
    return OPENVMS_USB_HID_OK;
}

openvms_usb_hid_status
openvms_usb_hid_find_boot_interface(const uint8_t *configuration,
                                   size_t length, uint8_t protocol,
                                   openvms_usb_hid_boot_interface *selected)
{
    /* Fixed-size bitmaps cover all wire-format values without allocation. */
    uint8_t alternate_seen[256][32] = {{0}};
    uint8_t interface_seen[32] = {0};
    uint8_t alternate_zero_seen[32] = {0};
    uint16_t endpoint_owner[32] = {0};
    interface_scan scan = {0};
    openvms_usb_hid_boot_interface match = {0};
    unsigned int interface_count = 0, zero_count = 0, match_count = 0;
    size_t offset;
    openvms_usb_hid_status status;

    if (configuration == NULL || selected == NULL ||
        (protocol != OPENVMS_USB_HID_KEYBOARD &&
         protocol != OPENVMS_USB_HID_MOUSE))
        return OPENVMS_USB_HID_INVALID_ARGUMENT;
    if (length < 9 || length > UINT16_MAX || configuration[0] != 9 ||
        configuration[1] != 2 ||
        little_endian_word(configuration + 2) != length ||
        configuration[4] == 0 || configuration[5] == 0 ||
        (configuration[7] & 0x80U) == 0 || (configuration[7] & 0x1fU) != 0)
        return OPENVMS_USB_HID_INVALID_DESCRIPTOR;

    offset = 9;
    while (offset < length) {
        const uint8_t *descriptor = configuration + offset;
        size_t descriptor_length;

        if (length - offset < 2)
            return OPENVMS_USB_HID_INVALID_DESCRIPTOR;
        descriptor_length = descriptor[0];
        if (descriptor_length < 2 || descriptor_length > length - offset)
            return OPENVMS_USB_HID_INVALID_DESCRIPTOR;

        if (descriptor[1] == 4) {
            const interface_scan empty = {0};
            uint8_t number, alternate;

            status = finish_interface(&scan, &match, &match_count);
            if (status != OPENVMS_USB_HID_OK)
                return status;
            if (descriptor_length != 9)
                return OPENVMS_USB_HID_INVALID_DESCRIPTOR;
            number = descriptor[2];
            alternate = descriptor[3];
            if (number >= configuration[4] ||
                !mark_bit(alternate_seen[number], alternate))
                return OPENVMS_USB_HID_INVALID_DESCRIPTOR;
            if (mark_bit(interface_seen, number))
                ++interface_count;
            if (alternate == 0 && mark_bit(alternate_zero_seen, number))
                ++zero_count;
            scan = empty;
            scan.present = 1;
            scan.is_hid = descriptor[5] == 3;
            scan.candidate = alternate == 0 && scan.is_hid &&
                             descriptor[6] == 1 && descriptor[7] == protocol;
            scan.expected_endpoints = descriptor[4];
            scan.value.configuration_value = configuration[5];
            scan.value.interface_number = number;
            scan.value.alternate_setting = alternate;
            scan.value.protocol = descriptor[7];
        } else if (descriptor[1] == 5) {
            uint8_t address;
            unsigned int index;
            uint32_t endpoint_bit;
            uint16_t owner, packet;

            if (!scan.present || descriptor_length < 7)
                return OPENVMS_USB_HID_INVALID_DESCRIPTOR;
            address = descriptor[2];
            if ((address & 0x70U) != 0 || (address & 0x0fU) == 0)
                return OPENVMS_USB_HID_INVALID_DESCRIPTOR;
            index = (address & 0x0fU) + ((address & 0x80U) ? 16U : 0U);
            endpoint_bit = UINT32_C(1) << index;
            owner = (uint16_t) (scan.value.interface_number + 1U);
            if ((scan.endpoints & endpoint_bit) != 0 ||
                (endpoint_owner[index] != 0 && endpoint_owner[index] != owner))
                return OPENVMS_USB_HID_INVALID_DESCRIPTOR;
            endpoint_owner[index] = owner;
            scan.endpoints |= endpoint_bit;
            ++scan.endpoint_count;
            if (scan.endpoint_count > scan.expected_endpoints)
                return OPENVMS_USB_HID_INVALID_DESCRIPTOR;

            if (scan.candidate) {
                packet = little_endian_word(descriptor + 4);
                if (descriptor_length != 7 || descriptor[3] != 3 ||
                    scan.hid_count != 1 || descriptor[6] == 0 ||
                    (packet & 0xf800U) != 0 || packet == 0 || packet > 1024)
                    return OPENVMS_USB_HID_INVALID_DESCRIPTOR;
                if ((address & 0x80U) != 0) {
                    unsigned int minimum = protocol == OPENVMS_USB_HID_KEYBOARD
                                           ? 8U : 3U;
                    if (packet < minimum)
                        return OPENVMS_USB_HID_INVALID_DESCRIPTOR;
                    ++scan.input_count;
                    scan.value.endpoint_address = address;
                    scan.value.max_packet_size = packet;
                    scan.value.interval = descriptor[6];
                }
            }
        } else if (descriptor[1] == 0x21 && scan.present && scan.is_hid) {
            if (scan.hid_count != 0 || scan.endpoint_count != 0 ||
                !valid_hid_descriptor(descriptor, descriptor_length))
                return OPENVMS_USB_HID_INVALID_DESCRIPTOR;
            ++scan.hid_count;
        } else if (descriptor[1] == 1 || descriptor[1] == 2) {
            return OPENVMS_USB_HID_INVALID_DESCRIPTOR;
        }
        offset += descriptor_length;
    }
    status = finish_interface(&scan, &match, &match_count);
    if (status != OPENVMS_USB_HID_OK)
        return status;
    if (interface_count != configuration[4] || zero_count != interface_count)
        return OPENVMS_USB_HID_INVALID_DESCRIPTOR;
    if (match_count == 0)
        return OPENVMS_USB_HID_NOT_FOUND;
    if (match_count != 1)
        return OPENVMS_USB_HID_AMBIGUOUS;
    *selected = match;
    return OPENVMS_USB_HID_OK;
}
