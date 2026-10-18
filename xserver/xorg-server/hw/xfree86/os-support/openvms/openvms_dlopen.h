/* OpenVMS image boundary for the ordinary Xorg dynamic loader. */
#ifndef OPENVMS_DLOPEN_H
#define OPENVMS_DLOPEN_H

#include <dlfcn.h>

/* OpenVMS CRTL does not provide the POSIX default-scope pseudo-handle. */
#ifndef RTLD_DEFAULT
#define RTLD_DEFAULT ((void *) 0)
#endif

int VmsXorgSetCoreImage(const char *image);
void *VmsXorgDlopen(const char *image, int flags);
void *VmsXorgDlsym(void *handle, const char *symbol);
int VmsXorgDlclose(void *handle);
char *VmsXorgDlerror(void);

/* Enable only for the unmodified upstream loader translation unit. Include
 * the vendor declarations first so their names and pointer pragmas survive. */
#ifdef VMS_XORG_REDIRECT_DLFCN
#define dlopen VmsXorgDlopen
#define dlsym VmsXorgDlsym
#define dlclose VmsXorgDlclose
#define dlerror VmsXorgDlerror
#endif

#endif
