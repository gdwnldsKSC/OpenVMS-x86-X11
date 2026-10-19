/* Original polling controller logic using public i8042/PS2 hardware commands.
 * Protocol references: independently acquired NetBSD i8042reg.h, pckbdreg.h,
 * and pmsreg.h. No OpenVMS implementation or console structure is used. */
#include "openvms_i8042.h"
#include <string.h>

enum {
    OUTPUT_FULL = 0x01,
    INPUT_FULL = 0x02,
    AUX_OUTPUT = 0x20,
    RECEIVE_ERROR = 0xc0,
    READ_CONTROL = 0x20,
    WRITE_CONTROL = 0x60,
    DISABLE_AUX = 0xa7,
    ENABLE_AUX = 0xa8,
    DISABLE_KEYBOARD = 0xad,
    ENABLE_KEYBOARD = 0xae,
    WRITE_AUX = 0xd4,
    DEVICE_DISABLE = 0xf5,
    DEVICE_ENABLE = 0xf4,
    ACK = 0xfa,
    RESEND = 0xfe,
    FAILURE = 0xfc,
    FAILURE_OTHER = 0xfd,
    WAIT_LIMIT = 200,
    DRAIN_LIMIT = 64,
    SEND_LIMIT = 3,
    BATCH_LIMIT = 32
};

static int input_empty(xorg_i8042_controller *c)
{
    unsigned i;
    for (i = 0; i < WAIT_LIMIT; ++i) {
        uint8_t status = 0;
        if (!c->ops->io(c->context, XORG_I8042_READ_STATUS, &status)) return 0;
        if (!(status & INPUT_FULL)) return 1;
        if (!c->ops->pause(c->context)) return 0;
    }
    return 0;
}

static int write_byte(xorg_i8042_controller *c, unsigned op, uint8_t value)
{
    if (!input_empty(c)) return 0;
    /* An unsuccessful I/O callback can still have reached the hardware. */
    c->touched = 1;
    return c->ops->io(c->context, op, &value) == 1;
}

static int command(xorg_i8042_controller *c, uint8_t value)
{
    return write_byte(c, XORG_I8042_WRITE_COMMAND, value);
}

static int drain(xorg_i8042_controller *c)
{
    unsigned i;
    for (i = 0; i <= DRAIN_LIMIT; ++i) {
        uint8_t status = 0, value = 0;
        if (!c->ops->io(c->context, XORG_I8042_READ_STATUS, &status)) return 0;
        if (!(status & OUTPUT_FULL)) return 1;
        if (i == DRAIN_LIMIT) return 0;
        /* This path discards input after inhibition. Even an errored byte
         * must be consumed to prove the output buffer is empty. Protocol
         * replies remain strict in reply(), and no drained data is delivered. */
        if (!c->ops->io(c->context, XORG_I8042_READ_DATA, &value)) return 0;
    }
    return 0;
}

static void inhibit_best_effort(xorg_i8042_controller *c)
{
    (void)command(c, DISABLE_KEYBOARD);
    (void)command(c, DISABLE_AUX);
}

static int fail(xorg_i8042_controller *c)
{
    c->failed = 1;
    c->active = 0;
    if (c->touched) inhibit_best_effort(c);
    return 0;
}

/* Only initialization calls this function. Bytes from a different source
 * can be stale data already in flight when the ports were inhibited. */
static int reply(xorg_i8042_controller *c, int aux, uint8_t *value)
{
    unsigned i;
    for (i = 0; i < WAIT_LIMIT; ++i) {
        uint8_t status = 0, data = 0;
        if (!c->ops->io(c->context, XORG_I8042_READ_STATUS, &status)) return 0;
        if (status & OUTPUT_FULL) {
            if (!c->ops->io(c->context, XORG_I8042_READ_DATA, &data)) return 0;
            if (status & RECEIVE_ERROR) return 0;
            if (((status & AUX_OUTPUT) != 0) == aux) {
                *value = data;
                return 1;
            }
        }
        if (!c->ops->pause(c->context)) return 0;
    }
    return 0;
}

static int device_byte(xorg_i8042_controller *c, int aux, uint8_t value)
{
    unsigned attempt;
    for (attempt = 0; attempt < SEND_LIMIT; ++attempt) {
        unsigned discarded;
        if (aux && !command(c, WRITE_AUX)) return 0;
        if (!write_byte(c, XORG_I8042_WRITE_DATA, value)) return 0;
        for (discarded = 0; discarded < DRAIN_LIMIT; ++discarded) {
            uint8_t response = 0;
            if (!reply(c, aux, &response)) return 0;
            if (response == ACK) return 1;
            if (response == RESEND) break;
            if (response == FAILURE || response == FAILURE_OTHER) return 0;
            /* A finite amount of stale stream data is permitted at setup. */
        }
        if (discarded == DRAIN_LIMIT) return 0;
    }
    return 0;
}

int xorg_i8042_start(xorg_i8042_controller *c,
                     const xorg_i8042_ops *ops, void *context)
{
    uint8_t control = 0, identity = 0;
    if (!c || !ops || !ops->io || !ops->batch || !ops->pause || c->attempted)
        return 0;
    c->ops = ops;
    c->context = context;
    c->attempted = 1;
    c->active = c->touched = c->failed = 0;
    if (!command(c, DISABLE_KEYBOARD) || !command(c, DISABLE_AUX) ||
        !drain(c) || !command(c, READ_CONTROL) || !reply(c, 0, &control))
        return fail(c);
    /* Preserve other command-byte bits, enable set-2 -> set-1 translation,
     * inhibit both ports, and disable both interrupts for polling ownership. */
    control = (uint8_t)((control | 0x70U) & ~0x03U);
    if (!command(c, WRITE_CONTROL) ||
        !write_byte(c, XORG_I8042_WRITE_DATA, control) ||
        !command(c, ENABLE_KEYBOARD) ||
        !device_byte(c, 0, DEVICE_DISABLE) ||
        !device_byte(c, 0, 0xf0) || !device_byte(c, 0, 0x02) ||
        !command(c, DISABLE_KEYBOARD) || !command(c, ENABLE_AUX) ||
        !device_byte(c, 1, DEVICE_DISABLE) ||
        !device_byte(c, 1, 0xf6) || !device_byte(c, 1, 0xf2) ||
        !reply(c, 1, &identity) || identity != 0 ||
        !device_byte(c, 1, 0xe6) ||
        !device_byte(c, 1, 0xf3) || !device_byte(c, 1, 100) ||
        !command(c, ENABLE_KEYBOARD) ||
        !device_byte(c, 0, DEVICE_ENABLE) ||
        !device_byte(c, 1, DEVICE_ENABLE))
        return fail(c);
    c->active = 1;
    return 1;
}

int xorg_i8042_stop(xorg_i8042_controller *c)
{
    int keyboard, aux, empty;
    if (!c) return 0;
    c->active = 0;
    if (!c->attempted || !c->touched) return 1;
    if (!c->ops || !c->ops->io || !c->ops->pause) return 0;
    keyboard = command(c, DISABLE_KEYBOARD);
    aux = command(c, DISABLE_AUX);
    empty = drain(c);
    if (!keyboard || !aux || !empty) {
        c->failed = 1;
        return 0;
    }
    c->touched = 0;
    return 1;
}

static void erase_pairs(xorg_i8042_pair *pairs, unsigned count)
{
    volatile unsigned char *bytes = (volatile unsigned char *)pairs;
    unsigned n = count * (unsigned)sizeof(*pairs);
    while (n--) *bytes++ = 0;
}

int xorg_i8042_read(xorg_i8042_controller *c, xorg_i8042_pair *pairs,
                    unsigned capacity, unsigned *count)
{
    xorg_i8042_pair pending[BATCH_LIMIT];
    unsigned i, received = 0;
    if (count) *count = 0;
    if (!c || !pairs || !count || !capacity || capacity > BATCH_LIMIT ||
        !c->active || c->failed || !c->ops || !c->ops->batch) return 0;
    if (!c->ops->batch(c->context, pending, capacity, &received) ||
        received > capacity) {
        erase_pairs(pending, BATCH_LIMIT);
        return fail(c);
    }
    for (i = 0; i < received; ++i) {
        if (!(pending[i].status & OUTPUT_FULL) ||
            (pending[i].status & RECEIVE_ERROR)) {
            erase_pairs(pending, BATCH_LIMIT);
            return fail(c);
        }
    }
    if (received) memcpy(pairs, pending, received * sizeof(*pairs));
    erase_pairs(pending, BATCH_LIMIT);
    *count = received;
    return 1;
}
