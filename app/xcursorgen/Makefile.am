bin_PROGRAMS = xcursorgen

xcursorgen_SOURCES = xcursorgen.c

AM_CFLAGS = $(CWARNFLAGS) $(XCURSORGEN_CFLAGS)

xcursorgen_LDADD = $(XCURSORGEN_LIBS)

appmandir = $(APP_MAN_DIR)

appman_PRE = xcursorgen.man
appman_DATA = $(appman_PRE:man=@APP_MAN_SUFFIX@)

EXTRA_DIST = $(appman_PRE) ChangeLog autogen.sh
CLEANFILES = $(appman_DATA)
MAINTAINERCLEANFILES = ChangeLog

.PHONY: ChangeLog

ChangeLog:
	$(CHANGELOG_CMD)

dist-hook: ChangeLog

# Strings to replace in man pages
XORGRELSTRING = @PACKAGE_STRING@
  XORGMANNAME = X Version 11

SED = sed

MAN_SUBSTS = \
	-e 's|__vendorversion__|"$(XORGRELSTRING)" "$(XORGMANNAME)"|' \
	-e 's|__appmansuffix__|$(APP_MAN_SUFFIX)|g' \
	-e 's|__libmansuffix__|$(LIB_MAN_SUFFIX)|g'

SUFFIXES = .$(APP_MAN_SUFFIX) .man

.man.$(APP_MAN_SUFFIX):
	$(AM_V_GEN)$(SED) $(MAN_SUBSTS) < $< > $@
