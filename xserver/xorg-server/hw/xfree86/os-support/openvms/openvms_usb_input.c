/* SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Kisara Development LLC
 * Input delivery and transport ownership stay on the ordinary server thread.
 */
#include "openvms_input.h"
#include "openvms_usb_hid.h"
#include "openvms_usb_ug.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct {
    openvms_usb_ug *keyboard;
    openvms_usb_ug *mouse;
    openvms_usb_hid_keyboard_state keys;
    unsigned int next_source;
    int attempted;
    int stopped;
    int failed;
    int sink_failed;
    int unsafe_cleanup;
} usb_input_source;

static usb_input_source source;

static void
erase_report(uint8_t *report, size_t length)
{
    volatile uint8_t *bytes = report;

    while (length != 0) {
        *bytes++ = 0;
        --length;
    }
}

static void
keyboard_event(void *context, uint8_t usage, int pressed)
{
    usb_input_source *input = context;

    if (!input->sink_failed && !OpenVMSKeyboardHID(usage, pressed))
        input->sink_failed = 1;
}

static void
mouse_event(void *context, uint8_t buttons, int8_t dx, int8_t dy)
{
    usb_input_source *input = context;

    if (!input->sink_failed && !OpenVMSPointerHID(buttons, dx, dy))
        input->sink_failed = 1;
}

static int
stop_source(void *context)
{
    usb_input_source *input = context;
    const openvms_usb_hid_keyboard_state empty = {0};
    int released;

    input->stopped = 1;
    if (input->keyboard != NULL) {
        if (!openvms_usb_ug_close(input->keyboard))
            input->unsafe_cleanup = 1;
        /* close invalidates either result; uncertain requests remain owned
         * by the transport until rundown, never by a reusable bridge handle. */
        input->keyboard = NULL;
    }
    if (input->mouse != NULL) {
        if (!openvms_usb_ug_close(input->mouse))
            input->unsafe_cleanup = 1;
        input->mouse = NULL;
    }
    if (openvms_usb_ug_cleanup_pending())
        input->unsafe_cleanup = 1;

    /* A sink can reject a transition partway through a valid report. The X
     * input owner releases its actual held state, including old keys absent
     * from the decoder's committed report. It also owns shutdown ordering:
     * individual event sinks cannot be called once input is stopping. */
    released = OpenVMSInputReleaseAll();
    input->keys = empty;
    return released && !input->unsafe_cleanup;
}

static int
lose_source(usb_input_source *input, const char *stage, const char *detail)
{
    if (!input->failed)
        fprintf(stderr, "XORG_USB_FAILURE stage=%s detail=%s\n", stage,
                detail != NULL ? detail : "unspecified");
    input->failed = 1;
    (void)stop_source(input);
    return -1;
}

static int
poll_source(void *context, unsigned int budget)
{
    usb_input_source *input = context;
    unsigned int attempts, reports = 0, idle = 0;

    if (input->failed || input->stopped)
        return -1;
    if (budget > OPENVMS_INPUT_REPORT_BUDGET)
        return lose_source(input, "poll", "invalid report budget");

    /* Alternate even when one source is pending. Bound transport calls as
     * well as completed reports, and stop after both sources have no data. */
    for (attempts = 0; reports < budget && attempts < budget * 2U; ++attempts) {
        uint8_t report[OPENVMS_USB_HID_MAX_BOOT_REPORT_SIZE] = {0};
        size_t length = 0;
        unsigned int keyboard = input->next_source == 0;
        openvms_usb_ug *transport = keyboard ? input->keyboard : input->mouse;
        openvms_usb_hid_status decoded;
        int result;

        input->next_source ^= 1U;
        if (transport == NULL) {
            if (++idle == 2)
                break;
            continue;
        }
        result = openvms_usb_ug_poll(transport, report, sizeof(report), &length);
        if (result < 0) {
            erase_report(report, sizeof(report));
            return lose_source(input, keyboard ? "keyboard-read" : "mouse-read",
                               openvms_usb_ug_error(transport));
        }
        if (result == 0) {
            erase_report(report, sizeof(report));
            if (++idle == 2)
                break;
            continue;
        }
        if (result != 1 || length > sizeof(report)) {
            erase_report(report, sizeof(report));
            return lose_source(input, "transport", "invalid report completion");
        }
        idle = 0;
        ++reports;
        input->sink_failed = 0;
        if (keyboard)
            decoded = openvms_usb_hid_keyboard_report(&input->keys, report,
                       length, keyboard_event, input);
        else
            decoded = openvms_usb_hid_mouse_report(report, length,
                                                 mouse_event, input);
        erase_report(report, sizeof(report));
        if (input->sink_failed)
            return lose_source(input, "event-delivery", "input sink refused event");
        if (keyboard && decoded == OPENVMS_USB_HID_KEYBOARD_ERROR)
            continue;
        if (decoded != OPENVMS_USB_HID_OK)
            return lose_source(input, keyboard ? "keyboard-report" : "mouse-report",
                               "invalid boot report");
    }
    return (int)reports;
}

int
OpenVMSUSBStart(void)
{
    const char *keyboard_device = getenv("XORG_OPENVMS_USB_KEYBOARD");
    const char *mouse_device = getenv("XORG_OPENVMS_USB_MOUSE");
    const char *stage;
    char error[256] = {0};

    if (source.attempted)
        return 0;
    source.attempted = 1;
    if (keyboard_device != NULL && keyboard_device[0] == '\0')
        keyboard_device = NULL;
    if (mouse_device != NULL && mouse_device[0] == '\0')
        mouse_device = NULL;
    if (keyboard_device == NULL && mouse_device == NULL) {
        fprintf(stderr, "XORG_USB_REFUSED explicit keyboard or mouse device required\n");
        return 0;
    }
    if (!OpenVMSInputReady() || !OpenVMSInputReleaseAll()) {
        fprintf(stderr, "XORG_USB_REFUSED input initialization\n");
        return 0;
    }
    /* Register before acquiring hardware so failed cleanup retains the
     * backend lease and prevents the input owner from retiring its devices. */
    if (!OpenVMSInputBackendRegister(poll_source, stop_source, &source)) {
        fprintf(stderr, "XORG_USB_REFUSED input backend registration\n");
        return 0;
    }
    stage = "keyboard-open";
    if (keyboard_device != NULL &&
        !openvms_usb_ug_open(&source.keyboard, keyboard_device,
                            OPENVMS_USB_HID_KEYBOARD, error, sizeof(error)))
        goto refused;
    stage = "mouse-open";
    if (mouse_device != NULL &&
        !openvms_usb_ug_open(&source.mouse, mouse_device,
                            OPENVMS_USB_HID_MOUSE, error, sizeof(error)))
        goto refused;
    fprintf(stderr, "XORG_USB_READY keyboard=%s mouse=%s boot-protocol\n",
            source.keyboard != NULL ? "enabled" : "disabled",
            source.mouse != NULL ? "enabled" : "disabled");
    return 1;

refused:
    error[sizeof(error) - 1] = '\0';
    fprintf(stderr, "XORG_USB_REFUSED stage=%s detail=%s\n", stage,
            error[0] != '\0' ? error : "transport open failed");
    source.failed = 1;
    if (!OpenVMSInputBackendStop())
        fprintf(stderr, "XORG_USB_CLEANUP retained until process rundown\n");
    return 0;
}
