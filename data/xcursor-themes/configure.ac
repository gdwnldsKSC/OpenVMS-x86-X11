AC_PREREQ([2.57])
AC_INIT(xcursor-themes, [1.0.2], [https://bugs.freedesktop.org/enter_bug.cgi?product=xorg], xcursor-themes)
AM_INIT_AUTOMAKE([dist-bzip2])
AM_MAINTAINER_MODE

# Require xorg-macros: XORG_CWARNFLAGS, XORG_CHANGELOG
m4_ifndef([XORG_MACROS_VERSION], [AC_FATAL([must install xorg-macros 1.2 or later before running autoconf/autogen])])
XORG_MACROS_VERSION(1.2)

AC_PROG_INSTALL

AC_PATH_PROG(XCURSORGEN, xcursorgen, no)
if test "x$XCURSORGEN" = xno; then
	AC_MSG_ERROR([xcursorgen is required to build these cursors.])
fi
AC_SUBST([XCURSORGEN])

PKG_CHECK_MODULES(ICONDEFS, xcursor)
pkg_cursordir=$(pkg-config --variable=icondir xcursor)
AC_ARG_WITH(cursordir,
        AC_HELP_STRING([--with-cursordir=<pathname>],
          [specify directory for cursor files (default is autodetected)]),
        [cursordir="$withval"], [cursordir="${pkg_cursordir}"])
AC_SUBST([cursordir])

AM_CONDITIONAL(WHITEGLASS, true)
AM_CONDITIONAL(REDGLASS, true)
AM_CONDITIONAL(HANDHELDS, true)

XORG_RELEASE_VERSION
XORG_CHANGELOG

AC_OUTPUT([
	Makefile
	handhelds/Makefile
	redglass/Makefile
	whiteglass/Makefile
])
