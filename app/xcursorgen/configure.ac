# $XdotOrg: app/xcursorgen/configure.ac,v 1.7 2005/12/15 00:24:05 kem Exp $

AC_PREREQ([2.57])
AC_INIT(xcursorgen, [1.0.3], [https://bugs.freedesktop.org/enter_bug.cgi?product=xorg], xcursorgen)
AM_INIT_AUTOMAKE([dist-bzip2])
AM_MAINTAINER_MODE

# Require xorg-macros 1.3 or later: XORG_DEFAULT_OPTIONS
m4_ifndef([XORG_MACROS_VERSION],
	  [m4_fatal([must install xorg-macros 1.3 or later before running autoconf/autogen])])
XORG_MACROS_VERSION(1.3)

AM_CONFIG_HEADER(config.h)

AC_PROG_CC
AC_PROG_INSTALL

XORG_DEFAULT_OPTIONS

# Checks for pkg-config packages
PKG_CHECK_MODULES(XCURSORGEN, x11 xcursor libpng12)
AC_SUBST(XCURSORGEN_CFLAGS)
AC_SUBST(XCURSORGEN_LIBS)

AC_OUTPUT([Makefile])
