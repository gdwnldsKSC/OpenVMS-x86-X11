#ifndef VMS_XPRINTAPPUTIL_NATIVE_H
#define VMS_XPRINTAPPUTIL_NATIVE_H

/* Request declarations from the installed SDK, without changing upstream. */
#include <string.h>
#include <strings.h>

/* Xfuncs.h supplies these macros after the SDK declarations are visible. */
#undef bcopy
#undef bzero
#undef bcmp

#endif
