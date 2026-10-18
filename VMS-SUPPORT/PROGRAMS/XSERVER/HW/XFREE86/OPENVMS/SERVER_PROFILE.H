/* Coherent feature layout for the native Xorg server and its internal modules. */
#ifndef OPENVMS_XORG_SERVER_PROFILE_H
#define OPENVMS_XORG_SERVER_PROFILE_H
#if !defined(__VMS) || __INITIAL_POINTER_SIZE != 64
#error Native OpenVMS with 64-bit pointers is required
#endif
#if defined(INPUTTHREAD) || defined(HAVE_SETITIMER)
#error This platform profile uses neither an input worker nor SIGALRM scheduling
#endif
#ifndef HAVE_DIX_CONFIG_H
#define HAVE_DIX_CONFIG_H 1
#endif
#define SYS_CONFIGDIRPATH ""
#include "DRIVER_PROFILE.H"
#endif
