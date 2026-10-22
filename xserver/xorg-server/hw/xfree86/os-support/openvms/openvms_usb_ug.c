/* SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Kisara Development LLC
 * References:
 * https://docs.vmssoftware.com/vsi-openvms-io-user-s-reference-manual/
 * Appendix F, and USB HID 1.11 sections 7.2 and Appendix B.
 */
#define __NEW_STARLET 1
#include <descrip.h>
#include <efndef.h>
#include <iodef.h>
#include <ssdef.h>
#include <starlet.h>
#include <sys$common:[syshlp.examples.usb]ugdef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "openvms_input.h"
#include "openvms_usb_hid.h"
#include "openvms_usb_ug.h"

#if !defined(__VMS) || !defined(__x86_64)
#error OPENVMS_X86_64_REQUIRED
#endif

/* A site may define OPENVMS_UG_USB_SUCCESS from its verified installed SDK
 * contract. Appendix F requires checking the second IOSB longword but does
 * not publish its success value. Without that, open refuses before
 * issuing QIO. Neither zero nor SS$_NORMAL is silently assumed to mean USB
 * success. The portable HID decoder remains independently usable.
 */

#define UG_WAIT_NS UINT64_C(1000000000)
#define UG_READ_NS UINT64_C(2000000000)
#define UG_WAIT_STEPS 1000U
#define UG_CONFIG_LIMIT 4096U

/* QIO has an eight-byte IOSB even for P64 callers. The second longword is
 * USB status, not the high word of a generic extended transfer count.
 */
typedef struct {
    volatile uint16_t status;
    volatile uint16_t count;
    volatile uint32_t usb_status;
} ug_iosb;
typedef char ug_iosb_must_be_8[sizeof(ug_iosb) == 8 ? 1 : -1];
typedef char ug_requires_p64[sizeof(void *) == 8 ? 1 : -1];

#pragma __required_pointer_size 32
typedef void *ug_ptr32;
typedef char *ug_char32;
/* The public C RTL 32-bit allocator is hidden by strict ANSI profiles.
 * Declare that documented interface with its actual short-pointer result.
 */
#ifndef _malloc32
extern void *_malloc32(size_t);
#endif
#pragma __required_pointer_size 64

typedef struct {
    ug_iosb iosb;
    uint64_t started;
    unsigned short channel;
    int active;
    int cancelling;
} ug_request;

struct openvms_usb_ug {
    struct dsc$descriptor_s device;
    char device_name[32];
    char error[160];
    unsigned short control_channel, input_channel;
    uint64_t control_pipe, input_pipe;
    ug_request control, input;
    union {
        uint64_t alignment;
        unsigned char bytes[UG_CONFIG_LIMIT];
    } work;
    unsigned char setup[8];
    unsigned char report[OPENVMS_USB_HID_MAX_BOOT_REPORT_SIZE];
    openvms_usb_hid_boot_interface interface;
    unsigned protocol;
    unsigned report_descriptor_length;
    unsigned char saved_protocol, saved_idle;
    int allocated, failed, restore_protocol, restore_idle;
};

static int incomplete_cleanup;

static void erase(void *buffer, size_t length)
{
    volatile unsigned char *p = buffer;
    while (length--) *p++ = 0;
}

static int fail(openvms_usb_ug *s, const char *stage, unsigned status)
{
    if (!s->failed)
        (void)snprintf(s->error, sizeof(s->error), "%s (status %08x)",
                       stage, status);
    s->failed = 1;
    return 0;
}

static int elapsed(uint64_t start, uint64_t duration)
{
    uint64_t now;
    return !OpenVMSMonotonicClock(&now) || now < start || now - start >= duration;
}

/* Only physical local names with an explicit controller and unit are accepted.
 * Leading '_' bypasses logical-name translation and generic allocation.
 */
static int device_name(openvms_usb_ug *s, const char *name)
{
    size_t n = 0, i;
    if (!name) return 0;
    if (*name == '_') ++name;
    while (name[n] && n < sizeof(s->device_name) - 2U) ++n;
    if (n < 5U || n >= sizeof(s->device_name) - 2U || name[n] ||
        name[n - 1U] != ':') return 0;
    s->device_name[0] = '_';
    for (i = 0; i < n; ++i) {
        unsigned char c = (unsigned char)name[i];
        if (c >= 'a' && c <= 'z') c = (unsigned char)(c - 'a' + 'A');
        s->device_name[i + 1U] = (char)c;
    }
    if (s->device_name[1] != 'U' || s->device_name[2] != 'G' ||
        s->device_name[3] < 'A' || s->device_name[3] > 'Z') return 0;
    for (i = 4; i < n; ++i)
        if (s->device_name[i] < '0' || s->device_name[i] > '9') return 0;
    s->device.dsc$w_length = (unsigned short)(n + 1U);
    s->device.dsc$b_dtype = DSC$K_DTYPE_T;
    s->device.dsc$b_class = DSC$K_CLASS_S;
    s->device.dsc$a_pointer = (ug_char32)s->device_name;
    return 1;
}

static int submit(openvms_usb_ug *s, ug_request *r, unsigned short channel,
                  unsigned function, void *p1, uint64_t p2, uint64_t p3,
                  uint64_t p4, uint64_t p5, uint64_t p6)
{
    unsigned status;
    if (r->active || !OpenVMSMonotonicClock(&r->started))
        return fail(s, "request state or monotonic clock", 0);
    memset((void *)&r->iosb, 0, sizeof(r->iosb));
    r->channel = channel;
    r->cancelling = 0;
    r->active = 1;
    status = sys$qio(EFN$C_ENF, channel, function,
                     (struct _iosb *)&r->iosb, 0, 0, p1,
                     p2, p3, p4, p5, p6);
    if (!(status & 1U)) {
        r->active = 0;
        return fail(s, "queue USB request", status);
    }
    return 1;
}

/* Storage referenced by QIO is always in s, never on a returning stack. */
static int cancel_request(openvms_usb_ug *s, ug_request *r)
{
    unsigned status, steps;
    uint64_t start;
    if (!r->active) return 1;
    if (!r->iosb.status) {
        status = sys$cancel(r->channel);
        if (!(status & 1U)) return fail(s, "cancel USB request", status);
        if (!OpenVMSMonotonicClock(&start)) return fail(s, "cancel clock", 0);
        for (steps = 0; !r->iosb.status && steps < UG_WAIT_STEPS; ++steps) {
            if (elapsed(start, UG_WAIT_NS)) break;
            (void)usleep(1000);
        }
    }
    if (!r->iosb.status) return fail(s, "USB cancellation incomplete", 0);
    r->active = 0;
    r->cancelling = 0;
    return 1;
}

static int completed(openvms_usb_ug *s, ug_request *r, int usb)
{
    unsigned status = r->iosb.status;
    if (!status) return 0;
    r->active = 0;
    if (!(status & 1U)) return fail(s, "USB I/O completion", status);
#ifdef OPENVMS_UG_USB_SUCCESS
    if (usb && r->iosb.usb_status != OPENVMS_UG_USB_SUCCESS)
        return fail(s, "USB bus completion", r->iosb.usb_status);
#else
    if (usb) return fail(s, "USB completion contract unavailable", 0);
#endif
    return 1;
}

static int wait_control(openvms_usb_ug *s, int usb)
{
    unsigned steps;
    for (steps = 0; !s->control.iosb.status && steps < UG_WAIT_STEPS; ++steps) {
        if (elapsed(s->control.started, UG_WAIT_NS)) break;
        (void)usleep(1000);
    }
    if (!s->control.iosb.status) {
        (void)fail(s, "USB setup deadline", 0);
        (void)cancel_request(s, &s->control);
        return 0;
    }
    return completed(s, &s->control, usb);
}

static int query(openvms_usb_ug *s, unsigned item, size_t length, uint64_t pipe)
{
    memset(s->work.bytes, 0, length);
    return submit(s, &s->control, s->control_channel, IO$_SENSEMODE,
                  s->work.bytes, length, item, pipe, 0, 0) && wait_control(s, 0);
}

static int associate(openvms_usb_ug *s, unsigned short channel, uint64_t *pipe)
{
    /* The installed, publicly documented UG example passes this operation's
     * handle by reference. Its storage must survive incomplete cancellation.
     * Other operations below pass the handle value, as Appendix F describes.
     */
    return submit(s, &s->control, channel, IO$_SETMODE, 0, 0,
                  UG$_ASSOCIATE, (uint64_t)(uintptr_t)pipe, 0, 0) && wait_control(s, 0);
}

/* Standard setup packet byte layout comes from USB 2.0 section 9.3. */
static int control(openvms_usb_ug *s, unsigned type, unsigned request,
                   unsigned value, unsigned index, unsigned length)
{
    if (length > sizeof(s->work.bytes)) return fail(s, "control length", 0);
    s->setup[0] = (unsigned char)type;
    s->setup[1] = (unsigned char)request;
    s->setup[2] = (unsigned char)value;
    s->setup[3] = (unsigned char)(value >> 8);
    s->setup[4] = (unsigned char)index;
    s->setup[5] = (unsigned char)(index >> 8);
    s->setup[6] = (unsigned char)length;
    s->setup[7] = (unsigned char)(length >> 8);
    if (type & 0x80U) memset(s->work.bytes, 0, length);
    if (!submit(s, &s->control, s->control_channel, IO$_SETMODE,
                s->setup, sizeof(s->setup), UG$_DEVICE_REQUEST,
                s->control_pipe,
                length ? (uint64_t)(uintptr_t)s->work.bytes : 0,
                USB$_SHORT_XFER_OK) || !wait_control(s, 1)) return 0;
    if (length && s->control.iosb.count != length)
        return fail(s, "short USB setup response", s->control.iosb.count);
    return 1;
}

/* Called only after the independent configuration validator succeeds.
 * Recover the selected interface's report-descriptor size from its HID
 * descriptor; a later GET_DESCRIPTOR obtains the report grammar itself.
 */
static int report_descriptor_size(openvms_usb_ug *s, size_t length)
{
    size_t offset = 0, entry;
    int selected = 0;
    while (offset + 2 <= length) {
        const unsigned char *d = s->work.bytes + offset;
        unsigned size = d[0];
        if (size < 2 || size > length - offset) return 0;
        if (d[1] == 4) {
            if (size < 9) return 0;
            selected = d[2] == s->interface.interface_number && d[3] == 0;
        } else if (selected && d[1] == 0x21) {
            if (size < 6 || (unsigned)d[5] * 3U > size - 6U) return 0;
            for (entry = 6; entry < 6U + (unsigned)d[5] * 3U; entry += 3)
                if (d[entry] == 0x22) {
                    s->report_descriptor_length = d[entry + 1] |
                        (unsigned)d[entry + 2] << 8;
                    return s->report_descriptor_length > 0 &&
                        s->report_descriptor_length <= UG_CONFIG_LIMIT;
                }
            return 0;
        }
        offset += size;
    }
    return 0;
}

/* HID 1.11 sections 6.2.2.2 and 6.2.2.7: reject Report ID globals and long
 * items. This establishes the limited single-ID profile needed to restore
 * one idle rate; it does not claim to implement a general report parser.
 */
static int single_report_id(const unsigned char *data, size_t length)
{
    size_t offset = 0;
    while (offset < length) {
        unsigned prefix = data[offset++], size = prefix & 3U;
        if (prefix == 0xfe || (prefix & 0xfcU) == 0x84U) return 0;
        if (size == 3) size = 4;
        if (size > length - offset) return 0;
        offset += size;
    }
    return length != 0;
}

static int discover(openvms_usb_ug *s)
{
    uint32_t count, type;
    uint64_t pipes[2];
    unsigned i, configuration_length;
    unsigned char bound_interface[9];
    if (!query(s, UG$_GET_PIPE_COUNT, sizeof(count), 0)) return 0;
    memcpy(&count, s->work.bytes, sizeof(count));
    if (count != 2) return fail(s, "requires exactly two UG pipes", count);
    if (!query(s, UG$_GET_PIPE_HANDLES, sizeof(pipes), 0)) return 0;
    memcpy(pipes, s->work.bytes, sizeof(pipes));
    if (!pipes[0] || !pipes[1] || pipes[0] == pipes[1])
        return fail(s, "invalid UG pipe handles", 0);
    for (i = 0; i < 2; ++i) {
        if (!query(s, UG$_GET_PIPE_TYPE, sizeof(type), pipes[i])) return 0;
        memcpy(&type, s->work.bytes, sizeof(type));
        if (type == UG$_PIPE_TYPE_CONTROL && !s->control_pipe)
            s->control_pipe = pipes[i];
        else if (type == UG$_PIPE_TYPE_INTERRUPT && !s->input_pipe)
            s->input_pipe = pipes[i];
        else return fail(s, "unsupported UG pipe combination", type);
    }
    if (!s->control_pipe || !s->input_pipe ||
        !associate(s, s->control_channel, &s->control_pipe)) return 0;
    if (!query(s, UG$_GET_INTERFACE_DESCRIPTOR, sizeof(bound_interface), 0)) return 0;
    memcpy(bound_interface, s->work.bytes, sizeof(bound_interface));
    if (bound_interface[0] != 9 || bound_interface[1] != 4 ||
        bound_interface[3] != 0 || bound_interface[4] != 1 ||
        bound_interface[5] != 3 || bound_interface[6] != 1 ||
        bound_interface[7] != s->protocol)
        return fail(s, "UG interface is not a single-endpoint boot interface", 0);
    if (!control(s, 0x80, 6, 0x0100, 0, 18)) return 0;
    if (s->work.bytes[0] != 18 || s->work.bytes[1] != 1 || s->work.bytes[17] != 1)
        return fail(s, "requires a single USB configuration", 0);
    if (!control(s, 0x80, 6, 0x0200, 0, 9)) return 0;
    configuration_length = s->work.bytes[2] | (unsigned)s->work.bytes[3] << 8;
    if (configuration_length < 9 || configuration_length > UG_CONFIG_LIMIT)
        return fail(s, "USB configuration size", configuration_length);
    if (!control(s, 0x80, 6, 0x0200, 0, configuration_length)) return 0;
    if (openvms_usb_hid_find_boot_interface(s->work.bytes, configuration_length,
            (uint8_t)s->protocol, &s->interface) != OPENVMS_USB_HID_OK ||
        s->interface.interface_number != bound_interface[2] ||
        s->interface.max_packet_size > sizeof(s->report) ||
        s->interface.interval > 16)
        return fail(s, "unsupported or ambiguous USB boot descriptor", 0);
    if (s->protocol == OPENVMS_USB_HID_KEYBOARD &&
        !report_descriptor_size(s, configuration_length))
        return fail(s, "keyboard report descriptor size", 0);
    /* These limits are valid at low, full and high speed. The OS schedules
     * the already-open pipe; the application does not convert bInterval. */
    if (!control(s, 0x80, 8, 0, 0, 1)) return 0;
    if (s->work.bytes[0] != s->interface.configuration_value)
        return fail(s, "active USB configuration differs", 0);
    if (!control(s, 0x81, 10, 0, s->interface.interface_number, 1)) return 0;
    if (s->work.bytes[0]) return fail(s, "active USB alternate differs", 0);
    if (s->protocol == OPENVMS_USB_HID_KEYBOARD) {
        if (!control(s, 0x81, 6, 0x2200, s->interface.interface_number,
                     s->report_descriptor_length)) return 0;
        if (!single_report_id(s->work.bytes, s->report_descriptor_length))
            return fail(s, "keyboard requires one report ID for idle restoration", 0);
    }
    return 1;
}

static int configure(openvms_usb_ug *s)
{
    unsigned index = s->interface.interface_number;
    if (!control(s, 0xa1, 3, 0, index, 1)) return 0;
    s->saved_protocol = s->work.bytes[0];
    if (s->saved_protocol > 1) return fail(s, "invalid original HID protocol", 0);
    if (s->protocol == OPENVMS_USB_HID_KEYBOARD) {
        if (!control(s, 0xa1, 2, 0, index, 1)) return 0;
        s->saved_idle = s->work.bytes[0];
    }
    /* Set restoration flags before submitting: a failed completion does not
     * prove that the device failed to execute the request. */
    s->restore_protocol = 1;
    if (!control(s, 0x21, 11, 0, index, 0)) return 0;
    if (s->protocol == OPENVMS_USB_HID_KEYBOARD) {
        s->restore_idle = 1;
        if (!control(s, 0x21, 10, 125U << 8, index, 0)) return 0;
    }
    return 1;
}

int openvms_usb_ug_open(openvms_usb_ug **result, const char *device,
                        unsigned protocol, char *error, size_t error_capacity)
{
    openvms_usb_ug *s;
    unsigned status;
    if (result) *result = NULL;
    if (error && error_capacity) error[0] = 0;
    if (!result || !error || !error_capacity) return 0;
    s = _malloc32(sizeof(*s));
    if (!s) {
        (void)snprintf(error, error_capacity, "USB request allocation failed");
        return 0;
    }
    if ((uintptr_t)s > UINT32_C(0x7fffffff) - sizeof(*s)) {
        free(s);
        (void)snprintf(error, error_capacity, "USB request storage is outside P0");
        return 0;
    }
    memset(s, 0, sizeof(*s));
    s->protocol = protocol;
    if ((protocol != OPENVMS_USB_HID_KEYBOARD && protocol != OPENVMS_USB_HID_MOUSE) ||
        !device_name(s, device)) {
        (void)fail(s, "explicit local UG device and boot protocol required", 0);
        goto refused;
    }
    status = sys$alloc((ug_ptr32)&s->device, 0, 0, 0, 0);
    /* DEVALRALLOC is success-class but belongs to an earlier owner. */
    if (status != SS$_NORMAL) {
        (void)fail(s, "exclusive UG device allocation", status);
        goto refused;
    }
    s->allocated = 1;
#ifndef OPENVMS_UG_USB_SUCCESS
    (void)fail(s, "UG USB completion contract is not configured in this build", 0);
    goto refused;
#endif
    status = sys$assign(&s->device, &s->control_channel, 0, 0);
    if (!(status & 1U)) {
        (void)fail(s, "assign UG control channel", status);
        goto refused;
    }
    if (!discover(s)) goto refused;
    status = sys$assign(&s->device, &s->input_channel, 0, 0);
    if (!(status & 1U)) {
        (void)fail(s, "assign UG input channel", status);
        goto refused;
    }
    if (!associate(s, s->input_channel, &s->input_pipe) || !configure(s)) goto refused;
    *result = s;
    return 1;
refused:
    (void)snprintf(error, error_capacity, "%s", s->error);
    if (!openvms_usb_ug_close(s)) {
        size_t used = strlen(error);
        if (used < error_capacity)
            (void)snprintf(error + used, error_capacity - used, "; cleanup incomplete");
    }
    return 0;
}

const char *openvms_usb_ug_error(const openvms_usb_ug *s)
{
    return s ? s->error : "USB source unavailable";
}

int openvms_usb_ug_cleanup_pending(void)
{
    return incomplete_cleanup;
}

int openvms_usb_ug_poll(openvms_usb_ug *s, unsigned char *report,
                        size_t capacity, size_t *length)
{
    unsigned status;
    size_t count;
    if (length) *length = 0;
    if (!s || s->failed) return -1;
    if (!report || !length || capacity < sizeof(s->report)) {
        (void)fail(s, "report buffer contract", 0);
        return -1;
    }
    if (!s->input.active) {
        erase(s->report, sizeof(s->report));
        if (!submit(s, &s->input, s->input_channel, IO$_READVBLK,
                    s->report, s->interface.max_packet_size,
                    USB$_SHORT_XFER_OK, s->input_pipe, 0, 0)) return -1;
    }
    status = s->input.iosb.status;
    if (!status) {
        if (s->input.cancelling) {
            if (elapsed(s->input.started, UG_WAIT_NS)) {
                (void)fail(s, "USB read cancellation deadline", 0);
                return -1;
            }
        } else if (elapsed(s->input.started, UG_READ_NS)) {
            /* SET_IDLE requests a keyboard heartbeat every 500 ms. A missed
             * heartbeat must release held input through the server bridge.
             * Mice legitimately remain silent while stationary.
             */
            if (s->protocol == OPENVMS_USB_HID_KEYBOARD) {
                (void)fail(s, "USB keyboard heartbeat deadline", 0);
                return -1;
            }
            status = sys$cancel(s->input_channel);
            if (!(status & 1U) || !OpenVMSMonotonicClock(&s->input.started)) {
                (void)fail(s, "cancel idle USB read", status);
                return -1;
            }
            s->input.cancelling = 1;
        }
        return 0;
    }
    if (s->input.cancelling && (status == SS$_CANCEL || status == SS$_ABORT)) {
        s->input.active = 0;
        s->input.cancelling = 0;
        erase(s->report, sizeof(s->report));
        return 0;
    }
    if (!completed(s, &s->input, 1)) return -1;
    s->input.cancelling = 0;
    count = s->input.iosb.count;
    if (count > s->interface.max_packet_size || count > capacity ||
        (s->protocol == OPENVMS_USB_HID_KEYBOARD ? count != 8 : count < 3)) {
        (void)fail(s, "invalid USB boot report length", (unsigned)count);
        erase(s->report, sizeof(s->report));
        return -1;
    }
    memcpy(report, s->report, count);
    *length = count;
    erase(s->report, sizeof(s->report));
    return 1;
}

static int close_source(openvms_usb_ug *s)
{
    int input_done, control_done, restored = 1;
    unsigned status;
    if (!s) return 1;
    input_done = cancel_request(s, &s->input);
    control_done = cancel_request(s, &s->control);
    if (!input_done || !control_done) return 0;
    if (s->restore_idle) {
        if (!control(s, 0x21, 10, (unsigned)s->saved_idle << 8,
                     s->interface.interface_number, 0)) restored = 0;
        else s->restore_idle = 0;
    }
    if (s->control.active) return 0;
    if (s->restore_protocol) {
        if (!control(s, 0x21, 11, s->saved_protocol,
                     s->interface.interface_number, 0)) restored = 0;
        else s->restore_protocol = 0;
    }
    if (s->control.active) return 0;
    if (!restored) return 0;
    if (s->input_channel) {
        status = sys$dassgn(s->input_channel);
        if (!(status & 1U)) return 0;
        s->input_channel = 0;
    }
    if (s->control_channel) {
        status = sys$dassgn(s->control_channel);
        if (!(status & 1U)) return 0;
        s->control_channel = 0;
    }
    if (s->allocated) {
        status = sys$dalloc((ug_ptr32)&s->device, 0);
        if (!(status & 1U)) return 0;
        s->allocated = 0;
    }
    erase(s, sizeof(*s));
    free(s);
    return 1;
}

int openvms_usb_ug_close(openvms_usb_ug *s)
{
    int result = close_source(s);
    if (!result) incomplete_cleanup = 1;
    return result;
}
