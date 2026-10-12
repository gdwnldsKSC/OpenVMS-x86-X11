windowswmdir = $(includedir)/X11/extensions
windowswm_HEADERS = \
        windowswm.h \
	windowswmstr.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = windowswmproto.pc

EXTRA_DIST = windowswmproto.pc.in

EXTRA_DIST += ChangeLog
MAINTAINERCLEANFILES = ChangeLog

.PHONY: ChangeLog

ChangeLog:
	$(CHANGELOG_CMD)

dist-hook: ChangeLog
