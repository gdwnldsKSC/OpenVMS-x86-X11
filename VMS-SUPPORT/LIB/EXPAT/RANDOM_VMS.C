/* Copyright (c) 2026 Kisara Development LLC. SPDX-License-Identifier: MIT */
#include "random_getentropy.h"
#include <errno.h>
#include <starlet.h>

/* Public SYS$GET_ENTROPY API: P64 buffer, at most 256 bytes, no privileges.
 * Implements Expat's entropy-backend interface without changing its parser.
 */
bool
writeRandomBytes_getentropy(void *target, size_t count)
{
    if (count == 0)
        return true;
    if (count <= 256 && (sys$get_entropy(target, count) & 1))
        return true;
    errno = EIO;
    return false;
}
