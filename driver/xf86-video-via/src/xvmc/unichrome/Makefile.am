
lib_LTLIBRARIES=libviaXvMC.la

libviaXvMC_la_SOURCES = \
	viaLowLevel.c \
	../driDrawable.c \
	../viaXvMC.c \
	../xf86dri.c

libviaXvMC_la_CFLAGS = @XORG_CFLAGS@ @DRI_CFLAGS@ -I$(top_srcdir)/src -I$(top_srcdir)/src/xvmc -DTRUE=1 -DFALSE=0
libviaXvMC_la_LDFLAGS = @DRI_LIBS@ -version-number 1:0:0
