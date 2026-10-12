lib_LTLIBRARIES = libWindowsWM.la

libWindowsWM_la_SOURCES = \
         windowswm.c

AM_CFLAGS = $(CWARNFLAGS) @WINDOWSWM_CFLAGS@ @MALLOC_ZERO_CFLAGS@
libWindowsWM_la_LIBADD = @WINDOWSWM_LIBS@

libWindowsWM_la_LDFLAGS = -version-info 7:0:0 -no-undefined
