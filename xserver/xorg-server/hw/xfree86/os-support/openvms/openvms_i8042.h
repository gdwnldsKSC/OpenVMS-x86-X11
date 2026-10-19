#ifndef OPENVMS_I8042_H
#define OPENVMS_I8042_H

#include <stdint.h>

enum xorg_i8042_operation {
    XORG_I8042_READ_STATUS = 1,
    XORG_I8042_READ_DATA = 2,
    XORG_I8042_WRITE_COMMAND = 3,
    XORG_I8042_WRITE_DATA = 4
};

typedef struct xorg_i8042_pair {
    uint8_t status;
    uint8_t data;
} xorg_i8042_pair;

typedef struct xorg_i8042_ops {
    int (*io)(void *context, unsigned operation, uint8_t *value);
    int (*batch)(void *context, xorg_i8042_pair *pairs, unsigned capacity,
                 unsigned *count);
    int (*pause)(void *context);
} xorg_i8042_ops;

typedef struct xorg_i8042_controller {
    const xorg_i8042_ops *ops;
    void *context;
    unsigned attempted;
    unsigned active;
    unsigned touched;
    unsigned failed;
} xorg_i8042_controller;

/* Zero-initialize before the first start. The callbacks must remain valid
 * through stop; pause waits approximately one millisecond. All callbacks and
 * public functions return one for success, zero for failure. A failed start
 * or read latches failure and attempts to inhibit both device ports. A
 * controller cannot be started twice without a successful stop followed by
 * caller zero-initialization. Before doing that the owner must release held
 * input and discard partial decoder state, while retaining exclusive ownership.
 * No operation resets the controller or restores firmware configuration. */
int xorg_i8042_start(xorg_i8042_controller *controller,
                     const xorg_i8042_ops *ops, void *context);
int xorg_i8042_stop(xorg_i8042_controller *controller);

/* One injected batch, preserving keyboard/AUX order and partial packets.
 * capacity must be 1..32. On failure count is zero and pairs is untouched. */
int xorg_i8042_read(xorg_i8042_controller *controller,
                    xorg_i8042_pair *pairs, unsigned capacity, unsigned *count);

#endif
