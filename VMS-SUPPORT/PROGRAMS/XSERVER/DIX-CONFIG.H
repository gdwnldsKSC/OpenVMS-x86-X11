#ifndef _DIX_CONFIG_H_
#define _DIX_CONFIG_H_

#if !defined(__VMS) || !defined(__INITIAL_POINTER_SIZE) || __INITIAL_POINTER_SIZE != 64
#error This configuration requires native OpenVMS with 64-bit pointers.
#endif

#define HAVE_DIRENT_H 1
#define HAVE_FCNTL_H 1
#define HAVE_INTTYPES_H 1
#define HAVE_STDINT_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRINGS_H 1
#define HAVE_STRING_H 1
#define HAVE_SYS_STAT_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_SYS_UTSNAME_H 1
#define HAVE_UNISTD_H 1
#define HAVE_SOCKLEN_T 1
#define HAVE_SIGPROCMASK 1
#define HAVE_POLL 1
#define HAVE_STRCASECMP 1
#define HAVE_STRNCASECMP 1
#define HAVE_STRNDUP 1
#define HAVE_CBRT 1
#define HAVE_TYPEOF 1
#define typeof __typeof__
#define NO_LOCAL_CLIENT_CRED 1
#define FONT_PATH_ELEMENT_NAME_CONST 1
#define _XTYPEDEF_POINTER 1
#define _XITYPEDEF_POINTER 1
#define X_BYTE_ORDER 1234
#define PACKAGE "xorg-server"
#define PACKAGE_NAME "xorg-server"
#define PACKAGE_VERSION "21.1.25"
#ifndef VENDOR_RELEASE
#define VENDOR_RELEASE 12101025
#endif
#ifndef XORG_VERSION_CURRENT
#define XORG_VERSION_CURRENT 12101025
#endif
#define XVENDORNAME "The X.Org Foundation"
#define BUILDERSTRING "OpenVMS x86-64"
#define BUILDERADDR "https://www.x.org/wiki/"
#define PROJECTROOT "."
#define SYSCONFDIR "."
#define SERVER_MISC_CONFIG_PATH "."
#define COMPILEDDEFAULTFONTPATH "built-ins"

/* Feature and path selections are supplied by the native module recipes. */
#endif
