#ifndef VMS_PCIACCESS_CONFIG_H
#define VMS_PCIACCESS_CONFIG_H

#if !defined(__VMS) || !defined(__INITIAL_POINTER_SIZE) || __INITIAL_POINTER_SIZE != 64
#error PCIACCESS requires native OpenVMS with 64-bit pointers.
#endif

#define HAVE_STRING_H 1
#define HAVE_STRINGS_H 1
#define HAVE_INTTYPES_H 1
#define HAVE_STDINT_H 1
#define PCIIDS_PATH "."

#endif
