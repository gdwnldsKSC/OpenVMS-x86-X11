/*
 * Copyright (c) 2026 Kisara Development LLC
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

/* First include for native P64 Xorg's upstream ospoll.c only. */
#ifndef OPENVMS_POLL_H
#define OPENVMS_POLL_H

#if !defined(__VMS) || !defined(__INITIAL_POINTER_SIZE) || __INITIAL_POINTER_SIZE != 64
#error "OPENVMS_POLL requires native OpenVMS /POINTER_SIZE=64"
#endif

#include "dix-config.h"
#include "xserver_poll.h"
#include <unistd.h>

#ifndef HAVE_POLL
#error "OPENVMS_POLL requires the native poll interface"
#endif

static int
OpenVMSPoll(struct pollfd *fds, nfds_t nfds, int timeout)
{
    /* Use a signal-interruptible wait for an empty infinite poll. */
    if (nfds == 0 && timeout < 0)
        return pause();
    return poll(fds, nfds, timeout);
}

#undef xserver_poll
#define xserver_poll(fds, nfds, timeout) OpenVMSPoll(fds, nfds, timeout)

#endif
