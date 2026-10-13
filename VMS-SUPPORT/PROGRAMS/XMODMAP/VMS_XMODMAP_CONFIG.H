#ifndef VMS_XMODMAP_CONFIG_H
#define VMS_XMODMAP_CONFIG_H

#define HAVE_STRNCASECMP 1
#define HAVE_ASPRINTF 1
/* VMS external names are case-insensitive; keep Exit distinct from exit. */
#define Exit XmodmapExit
#define PACKAGE "xmodmap"
#define PACKAGE_NAME "xmodmap"
#define PACKAGE_TARNAME "xmodmap"
#define PACKAGE_VERSION "1.0.7"
#define PACKAGE_STRING "xmodmap 1.0.7"
#define PACKAGE_BUGREPORT "https://bugs.freedesktop.org/enter_bug.cgi?product=xorg"
#define PACKAGE_VERSION_MAJOR 1
#define PACKAGE_VERSION_MINOR 0
#define PACKAGE_VERSION_PATCHLEVEL 7
#define VERSION "1.0.7"

#endif
