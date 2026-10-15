#ifndef VMS_XFONT2_CONFIG_H
#define VMS_XFONT2_CONFIG_H

#if !defined(__VMS) || !defined(__INITIAL_POINTER_SIZE) || __INITIAL_POINTER_SIZE != 64
#error XFONT2 requires native OpenVMS with 64-bit pointers.
#endif

#define PACKAGE "libXfont2"
#define PACKAGE_VERSION "2.0.9"
#define VERSION PACKAGE_VERSION
#define HAVE_FLOAT_H 1
#define HAVE_INTTYPES_H 1
#define HAVE_STDINT_H 1
#define HAVE_STDIO_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRING_H 1
#define HAVE_STRINGS_H 1
#define HAVE_SYS_STAT_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_UNISTD_H 1
#define HAVE_READLINK 1
#define HAVE_STRCASECMP 1
#define HAVE_SOCKLEN_T 1
#define HAVE_STRUCT_SOCKADDR_STORAGE 1
#define STDC_HEADERS 1
#define XFONT_BITMAP 1
#define XFONT_BDFFORMAT 1
#define XFONT_PCFFORMAT 1
#define XFONT_SNFFORMAT 1
#define XFONT_BUILTINS 1
#define XFONT_FREETYPE 1
#define XFONT_FONTFILE 1
#define XFONT_FC 1
#define X_GZIP_FONT_COMPRESSION 1
#define TCPCONN 1
#define OPEN_MAX 256

#endif

