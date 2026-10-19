/* Original process-owned i8042 adapter using public hardware and VMS services.
 * For a serial-OPA0 VMware guest with no competing controller owner. A process
 * crash may leave input inhibited; reboot is the recovery procedure. */
#define __NEW_STARLET 1
#include <descrip.h>
#include <lckdef.h>
#include <ssdef.h>
#include <starlet.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "openvms_i8042.h"
#include "openvms_input.h"

#if !defined(__VMS) || !defined(__x86_64)
#error OPENVMS_X86_64_REQUIRED
#endif
#define CALLBACK_STATUS UINT32_C(0x05A20009)
#define BATCH_OPERATION 5U
#define BATCH_LIMIT 32U
#define RECOVERY_LIMIT 8U
#define RECOVERY_WINDOW_NS UINT64_C(10000000000)
#define RECOVERY_DEADLINE_NS UINT64_C(2000000000)
typedef char require_pointer64[sizeof(void *) == 8 ? 1 : -1];
extern uint32_t XORG_I8042_PIO(uint32_t operation, uint32_t value);
static volatile uint32_t io_operation, io_value, io_entered, io_count;
enum io_fault_code {
    IO_FAULT_NONE, IO_FAULT_SERVICE, IO_FAULT_CAPACITY, IO_FAULT_STATUS,
    IO_FAULT_PARITY, IO_FAULT_TIMEOUT, IO_FAULT_DATA, IO_FAULT_LOCK,
    IO_FAULT_COUNT
};
static volatile uint32_t io_fault;
static uint32_t batch_fault;
static xorg_i8042_pair io_pairs[BATCH_LIMIT];
static unsigned __int64 no_arguments[1] = {0};
static struct {
    uint16_t status, reserved;
    uint32_t id;
} lock_status;
static $DESCRIPTOR(lock_name, "XORG$VMWARE$I8042");
static $DESCRIPTOR(old_device, "PXA0:");
static struct {
    int attempted, stopped, failed;
    int recovering;
    uint32_t keyboard_bytes, mouse_bytes;
    unsigned recovery_attempts, recoveries;
    uint64_t recovery_window, recovery_start;
    xorg_i8042_controller controller;
} source;

/* No waits, allocation, X/CRTL calls, or unbounded loops in kernel context. */
static int kernel_io(void)
{
    uint32_t i, capacity = io_value;
    io_entered = 1;
    if (io_operation != BATCH_OPERATION) {
        io_value = XORG_I8042_PIO(io_operation, io_value);
        return (int)CALLBACK_STATUS;
    }
    io_count = 0;
    if (!capacity || capacity > BATCH_LIMIT) {
        io_fault = IO_FAULT_CAPACITY;
        return 0;
    }
    for (i = 0; i < capacity; ++i) {
        uint32_t status = XORG_I8042_PIO(XORG_I8042_READ_STATUS, 0);
        uint32_t data;
        if (status > UINT8_MAX) {
            io_fault = IO_FAULT_STATUS;
            return 0;
        }
        if (!(status & 1U)) break;
        data = XORG_I8042_PIO(XORG_I8042_READ_DATA, 0);
        if (data > UINT8_MAX) {
            io_fault = IO_FAULT_DATA;
            return 0;
        }
        /* Consume an errored available byte, but never deliver it. Recovery
         * discards this whole batch and re-establishes both stream profiles. */
        if (status & 0xc0U) {
            io_fault = status & 0x80U ? IO_FAULT_PARITY : IO_FAULT_TIMEOUT;
            return 0;
        }
        io_pairs[i].status = (uint8_t)status;
        io_pairs[i].data = (uint8_t)data;
        io_count = i + 1U;
    }
    return (int)CALLBACK_STATUS;
}

static void erase(void *buffer, size_t length)
{
    volatile unsigned char *p = buffer;
    while (length--) *p++ = 0;
}

static int invoke(uint32_t operation, uint32_t value)
{
    unsigned int status;
    io_operation = operation;
    io_value = value;
    io_entered = 0;
    io_fault = IO_FAULT_NONE;
    status = (unsigned int)sys$cmkrnl_64(kernel_io, no_arguments);
    if ((status != CALLBACK_STATUS || io_entered != 1U) && !io_fault)
        io_fault = IO_FAULT_SERVICE;
    return status == CALLBACK_STATUS && io_entered == 1U;
}

static int recovery_time_available(void)
{
    uint64_t now;
    if (!source.recovering) return 1;
    return OpenVMSMonotonicClock(&now) && now >= source.recovery_start &&
           now - source.recovery_start < RECOVERY_DEADLINE_NS;
}

static int controller_io(void *context, unsigned operation, uint8_t *value)
{
    int ok;
    uint32_t result;
    (void)context;
    if (!value || operation < XORG_I8042_READ_STATUS ||
        operation > XORG_I8042_WRITE_DATA || !lock_status.id ||
        !recovery_time_available()) return 0;
    ok = invoke(operation, *value);
    result = io_value;
    io_value = 0;
    if (!ok || result > UINT8_MAX) return 0;
    *value = (uint8_t)result;
    return 1;
}

static int controller_batch(void *context, xorg_i8042_pair *pairs,
                            unsigned capacity, unsigned *count)
{
    int ok;
    (void)context;
    *count = 0;
    io_fault = IO_FAULT_LOCK;
    ok = lock_status.id && invoke(BATCH_OPERATION, capacity) && io_count <= capacity;
    if (!ok && !io_fault) io_fault = IO_FAULT_COUNT;
    batch_fault = io_fault;
    if (ok) {
        *count = io_count;
        memcpy(pairs, io_pairs, *count * sizeof(*pairs));
    }
    erase(io_pairs, sizeof(io_pairs));
    io_count = 0;
    return ok;
}

static int controller_pause(void *context)
{
    (void)context;
    if (!recovery_time_available()) return 0;
    return usleep(1000U) == 0 || errno == EINTR;
}
static const xorg_i8042_ops controller_ops = {
    controller_io, controller_batch, controller_pause
};

static int unlock(void)
{
    unsigned int status;
    if (!lock_status.id) return 1;
    status = sys$deq(lock_status.id, 0, 0, 0);
    if (!(status & 1U)) return 0;
    memset(&lock_status, 0, sizeof(lock_status));
    return 1;
}

static int stop_source(void *context)
{
    int inhibited, released;
    (void)context;
    source.stopped = 1;
    inhibited = xorg_i8042_stop(&source.controller);
    released = OpenVMSInputReleaseAll();
    /* Retain lock on uncertain cleanup until process rundown. */
    return inhibited && released && unlock();
}

static const char *transport_fault(void)
{
    switch (batch_fault) {
    case IO_FAULT_NONE: return "none";
    case IO_FAULT_SERVICE: return "kernel-service";
    case IO_FAULT_CAPACITY: return "batch-capacity";
    case IO_FAULT_STATUS: return "status-range";
    case IO_FAULT_PARITY: return "parity";
    case IO_FAULT_TIMEOUT: return "timeout";
    case IO_FAULT_DATA: return "data-range";
    case IO_FAULT_LOCK: return "ownership-lock";
    case IO_FAULT_COUNT: return "batch-count";
    default: return "unknown";
    }
}

static int lose_source(const char *reason)
{
    /* One aggregate failure record, before teardown erases held input. Never
     * log scan codes, packet bytes, key identities, buttons, or coordinates. */
    if (!source.failed)
        fprintf(stderr, "XORG_I8042_FAILURE stage=%s transport=%s "
                "keyboard_bytes=%lu mouse_bytes=%lu\n", reason, transport_fault(),
                (unsigned long)source.keyboard_bytes,
                (unsigned long)source.mouse_bytes);
    source.failed = 1;
    (void)stop_source(NULL);
    return -1;
}

static int recover_source(void)
{
    uint64_t now;
    int ok = 0;
    if (!lock_status.id || source.failed || source.stopped ||
        (batch_fault != IO_FAULT_PARITY && batch_fault != IO_FAULT_TIMEOUT) ||
        !OpenVMSMonotonicClock(&now)) return 0;
    if (source.recovery_attempts && now < source.recovery_window) return 0;
    if (!source.recovery_attempts ||
        now - source.recovery_window >= RECOVERY_WINDOW_NS) {
        source.recovery_window = now;
        source.recovery_attempts = 0;
    }
    if (source.recovery_attempts >= RECOVERY_LIMIT) return 0;
    ++source.recovery_attempts;
    source.recovery_start = now;
    source.recovering = 1;
    /* Keep the ownership lock throughout. Do not clear uncertain hardware
     * state, or reset upstream parsers before all held X inputs are released. */
    if (!xorg_i8042_stop(&source.controller) || !OpenVMSInputReleaseAll()) goto done;
    memset(&source.controller, 0, sizeof(source.controller));
    if (!xorg_i8042_start(&source.controller, &controller_ops, NULL) ||
        !recovery_time_available()) goto done;
    if (source.recoveries != UINT32_MAX) ++source.recoveries;
    fprintf(stderr, "XORG_I8042_RECOVERED transport=%s recoveries=%u window_attempts=%u\n",
            transport_fault(), source.recoveries, source.recovery_attempts);
    ok = 1;
done:
    /* On failure the caller runs normal fail-closed cleanup outside the
     * expired recovery deadline, so inhibition still gets a bounded attempt. */
    source.recovering = 0;
    return ok;
}

static int poll_source(void *context, unsigned budget)
{
    xorg_i8042_pair pairs[BATCH_LIMIT];
    unsigned count = 0, i;
    int ok;
    const char *reason = "transport";
    (void)context;
    if (source.failed || source.stopped || budget > BATCH_LIMIT || !budget)
        return lose_source("poll-state-or-budget");
    batch_fault = IO_FAULT_NONE;
    ok = xorg_i8042_read(&source.controller, pairs, budget, &count);
    if (!ok && (batch_fault == IO_FAULT_PARITY || batch_fault == IO_FAULT_TIMEOUT)) {
        erase(pairs, sizeof(pairs));
        return recover_source() ? 0 : lose_source("receive-recovery");
    }
    for (i = 0; ok && i < count; ++i) {
        if (pairs[i].status & 0x20U) {
            reason = "upstream-mouse-input";
            ok = OpenVMSPointerByte(pairs[i].data);
            if (ok && source.mouse_bytes != UINT32_MAX) ++source.mouse_bytes;
        } else {
            reason = "upstream-keyboard-input";
            ok = OpenVMSKeyboardByte(pairs[i].data);
            if (ok && source.keyboard_bytes != UINT32_MAX) ++source.keyboard_bytes;
        }
    }
    erase(pairs, sizeof(pairs));
    return ok ? (int)count : lose_source(reason);
}

int OpenVMSI8042Start(void)
{
    const char *assertion = getenv("XORG_OPENVMS_INPUT");
    const char *stage = "exclusive serial-console launch assertion";
    unsigned int status;
    unsigned short channel = 0;
    if (source.attempted) return 0;
    source.attempted = 1;
    if (!assertion || strcmp(assertion, "I8042_SERIAL_CONSOLE")) goto refused;
    stage = "legacy input device must be absent";
    status = sys$assign(&old_device, &channel, 0, 0);
    if (status & 1U) {
        if (channel) (void)sys$dassgn(channel);
        goto refused;
    }
    if (status != SS$_NOSUCHDEV) goto refused;
    stage = "exclusive cooperating process lock";
    status = sys$enqw(0, LCK$K_EXMODE, (struct _lksb *)&lock_status,
                     LCK$M_NOQUEUE | LCK$M_SYSTEM, &lock_name,
                     0, 0, 0, 0, 0, 0);
    if (!(status & 1U) || !(lock_status.status & 1U) || !lock_status.id) goto refused;
    stage = "X input initialization";
    if (!OpenVMSInputReady() || !OpenVMSInputReleaseAll())
        goto refused;
    stage = "bounded i8042 negotiation";
    if (!xorg_i8042_start(&source.controller, &controller_ops, NULL)) goto refused;
    stage = "input backend registration";
    if (!OpenVMSInputBackendRegister(poll_source, stop_source, NULL)) goto refused;
    fprintf(stderr, "XORG_I8042_READY translated-set1 standard-three-byte-mouse\n");
    return 1;
refused:
    fprintf(stderr, "XORG_I8042_REFUSED %s\n", stage);
    (void)stop_source(NULL);
    return 0;
}
