/* Copyright (c) 2026 Kisara Development LLC. */
/* Prefix for the generated zconf.h; the upstream header follows unchanged. */
#ifndef KISARA_ZCONF_VMS_H
#define KISARA_ZCONF_VMS_H
#ifdef __VMS
/* Keep the public offset ABI independent of callers' HAVE_UNISTD_H setting. */
#ifndef z_off_t
#define z_off_t long long
#endif
#include <unistd.h>
#include <unixio.h>
#endif
#endif

