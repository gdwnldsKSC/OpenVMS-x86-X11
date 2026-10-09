
lib_LTLIBRARIES=libviaXvMCPro.la

libviaXvMCPro_la_SOURCES = \
	viaLowLevelPro.c \
	../driDrawable.c \
	../viaXvMC.c \
	../xf86dri.c

libviaXvMCPro_la_CFLAGS = @XORG_CFLAGS@ @DRI_CFLAGS@ -I$(top_srcdir)/src -I$(top_srcdir)/src/xvmc -DTRUE=1 -DFALSE=0
libviaXvMCPro_la_LDFLAGS = @DRI_LIBS@ -version-number 1:0:0
