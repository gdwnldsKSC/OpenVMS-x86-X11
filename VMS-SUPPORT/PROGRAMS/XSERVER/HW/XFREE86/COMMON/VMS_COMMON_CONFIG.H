#ifndef VMS_COMMON_CONFIG_H
#define VMS_COMMON_CONFIG_H

/* Native build configuration only; platform services are supplied at link. */
#if !defined(__VMS) || !defined(__INITIAL_POINTER_SIZE) || __INITIAL_POINTER_SIZE != 64
#error COMMON and INIT require native OpenVMS with 64-bit pointers.
#endif
#ifndef XFree86Server
#define XFree86Server 1
#endif
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>

#ifndef __XSERVERNAME__
#define __XSERVERNAME__ "Xorg"
#endif
#ifndef __VENDORDWEBSUPPORT__
#define __VENDORDWEBSUPPORT__ "https://www.x.org"
#endif
#ifndef XVENDORNAME
#define XVENDORNAME "The X.Org Foundation"
#endif
#ifndef BUILDERADDR
#define BUILDERADDR "xorg@lists.freedesktop.org"
#endif
#ifndef XF86CONFIGFILE
#define XF86CONFIGFILE "xorg.conf"
#endif
#ifndef __XCONFIGFILE__
#define __XCONFIGFILE__ XF86CONFIGFILE
#endif
#ifndef DEFAULT_MODULE_PATH
#define DEFAULT_MODULE_PATH "./lib/xorg/modules"
#endif
#ifndef DEFAULT_LOGPREFIX
#define DEFAULT_LOGPREFIX "./Xorg."
#endif
#ifndef PROJECTROOT
#define PROJECTROOT "."
#endif
#ifndef DRIVERS
#define DRIVERS ""
#endif
#ifndef IDRIVERS
#define IDRIVERS ""
#endif
#ifndef __XKBDEFRULES__
#define __XKBDEFRULES__ "xorg"
#endif
#ifndef XORG_RELEASE
#define XORG_RELEASE "Release 7.1"
#endif
#ifndef ARCH_PCI_INIT
#define ARCH_PCI_INIT vmsPciInit
#endif

#endif
