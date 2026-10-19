/* Installed C RTL monotonic clock, compiled in its native feature profile. */
#if defined(__VMS) && !defined(_XOPEN_SOURCE_EXTENDED)
#define _XOPEN_SOURCE_EXTENDED 1
#endif
#include "openvms_input.h"
#include <time.h>

#ifndef CLOCK_MONOTONIC
#error This provider requires a real CLOCK_MONOTONIC implementation
#endif

int OpenVMSMonotonicClock(uint64_t *nanoseconds)
{
    struct timespec now;
    uint64_t seconds;
    if (!nanoseconds || clock_gettime(CLOCK_MONOTONIC, &now) != 0)
        return 0;
    /* A negative signed tv_sec also becomes too large after conversion. */
    seconds = (uint64_t)now.tv_sec;
    if (now.tv_nsec < 0 || now.tv_nsec >= 1000000000L ||
        seconds > (UINT64_MAX - (uint64_t)now.tv_nsec) / UINT64_C(1000000000))
        return 0;
    *nanoseconds = seconds * UINT64_C(1000000000) + (uint64_t)now.tv_nsec;
    return 1;
}
