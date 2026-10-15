#ifndef _XORG_SERVER_H_
#define _XORG_SERVER_H_

/* Server features and version are selected by the native driver recipes. */
#ifdef HAVE_XORG_CONFIG_H
#error Include xorg-config.h when building the X server
#endif
#define X_BYTE_ORDER 1234
#define FONT_PATH_ELEMENT_NAME_CONST 1
#define HAVE_TYPEOF 1
#define typeof __typeof__
#include <X11/Xfuncproto.h>

#endif
