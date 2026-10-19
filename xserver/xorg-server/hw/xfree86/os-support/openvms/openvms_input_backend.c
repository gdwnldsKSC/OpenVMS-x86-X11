/* Original ordinary-context native-server backend registration/lifecycle.
 * Deliberately no CRTL device calls, automatic registration, or fake provider.
 */
#include "openvms_input.h"
#include <stddef.h>

static struct {
    openvms_input_poll_fn poll;
    openvms_input_stop_fn stop;
    void *context;
    int busy, failed, stopped;
} native_backend;

int OpenVMSInputBackendRegister(openvms_input_poll_fn poll,
                              openvms_input_stop_fn stop, void *context)
{
    if (!poll || !stop || native_backend.poll || native_backend.busy ||
        native_backend.failed || native_backend.stopped) return 0;
    native_backend.poll = poll;
    native_backend.stop = stop;
    native_backend.context = context;
    return 1;
}

int OpenVMSInputBackendActive(void)
{
    return native_backend.poll != NULL && !native_backend.stopped;
}

int OpenVMSInputBackendPoll(void)
{
    int result;
    if (native_backend.busy || native_backend.failed) return -1;
    if (!native_backend.poll || native_backend.stopped) return 0;
    native_backend.busy = 1;
    result = (*native_backend.poll)(native_backend.context, OPENVMS_INPUT_REPORT_BUDGET);
    native_backend.busy = 0;
    if (result < 0 || (unsigned int)result > OPENVMS_INPUT_REPORT_BUDGET) {
        native_backend.failed = 1;
        return -1;
    }
    return result;
}

int OpenVMSInputBackendStop(void)
{
    int result;
    if (native_backend.busy) return 0;
    if (native_backend.stopped) return 1;
    if (!native_backend.poll) {
        native_backend.stopped = 1;
        return 1;
    }
    native_backend.busy = 1;
    result = (*native_backend.stop)(native_backend.context);
    native_backend.busy = 0;
    if (result != 1) {
        native_backend.failed = 1;
        return 0;
    }
    native_backend.stopped = 1;
    native_backend.poll = NULL;
    native_backend.stop = NULL;
    native_backend.context = NULL;
    return 1;
}
