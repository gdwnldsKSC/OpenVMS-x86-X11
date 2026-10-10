#ifndef VMS_XKBFILE_NATIVE_H
#define VMS_XKBFILE_NATIVE_H

/* Obtain native P64 declarations from the installed SDK before X headers. */
#include <string.h>
#include <strings.h>

/* Xfuncs.h supplies the upstream legacy aliases after these declarations. */
#undef bcopy
#undef bzero
#undef bcmp

#endif
