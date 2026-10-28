/* Upstream fbdev uses the same SDK layout as the other native drivers. */
#ifndef OPENVMS_XORG_FBDEV_PROFILE_H
#define OPENVMS_XORG_FBDEV_PROFILE_H
#include "DRIVER_PROFILE.H"
#include <xorg-server.h>
/* DGA assumes /dev/mem and a long-sized physical address; neither is an
 * established interface for this driver on OpenVMS.
 */
#undef XFreeXDGA
#define XFree86LOADER 1
#define PACKAGE_VERSION_MAJOR 0
#define PACKAGE_VERSION_MINOR 5
#define PACKAGE_VERSION_PATCHLEVEL 1
#endif
