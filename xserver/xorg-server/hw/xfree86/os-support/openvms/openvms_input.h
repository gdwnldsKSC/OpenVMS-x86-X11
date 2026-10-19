#ifndef OPENVMS_INPUT_H
#define OPENVMS_INPUT_H

#include <stdint.h>

/* One ordinary-server-context owner, enabled explicitly by the console path.
 * Upstream kbd/mouse own decoding, XKB maps and event delivery. Hardware stop
 * must complete before the devices or their master pointers are retired. */
void OpenVMSInputRegister(void);
int OpenVMSInputStart(void);
int OpenVMSInputStop(void);
int OpenVMSInputReady(void);
int OpenVMSInputReleaseAll(void);
int OpenVMSKeyboardByte(unsigned char value);
int OpenVMSPointerByte(unsigned char value);
int OpenVMSI8042Start(void);
int OpenVMSMonotonicClock(uint64_t *nanoseconds);

/* This registry owns no device and has no default provider. Both callbacks
 * run only in the serialized server loop; never from a signal, AST or thread.
 * Stop inhibits acquisition and discards stale bytes before input teardown.
 * A failed stop retains the hardware lease until process rundown. */
#define OPENVMS_INPUT_REPORT_BUDGET 32U
typedef int (*openvms_input_poll_fn)(void *context, unsigned int report_budget);
typedef int (*openvms_input_stop_fn)(void *context);
int OpenVMSInputBackendRegister(openvms_input_poll_fn poll,
                               openvms_input_stop_fn stop, void *context);
int OpenVMSInputBackendActive(void);
int OpenVMSInputBackendPoll(void);
int OpenVMSInputBackendStop(void);

#endif
