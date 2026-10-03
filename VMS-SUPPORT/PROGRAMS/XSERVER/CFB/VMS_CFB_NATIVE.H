#ifndef VMS_CFB_NATIVE_H
#define VMS_CFB_NATIVE_H

#include <stdint.h>
#include <string.h>
#include <strings.h>

/* Upstream Xfuncs.h supplies these legacy aliases for the CFB profile. */
#undef bcopy
#undef bzero

static inline uint32_t vms_cfb_load_u32(const void *source)
{
    uint32_t value;
    memcpy(&value, source, sizeof value);
    return value;
}

#define ldl_u(p) vms_cfb_load_u32(p)

#endif
