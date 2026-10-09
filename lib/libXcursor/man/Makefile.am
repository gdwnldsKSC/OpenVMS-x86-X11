libmandir = $(LIB_MAN_DIR)

libman_PRE = Xcursor.man
libman_DATA = $(libman_PRE:man=@LIB_MAN_SUFFIX@)

all-local: $(libman_DATA)

EXTRA_DIST = $(libman_PRE)
CLEANFILES = $(libman_DATA)

SUFFIXES = .$(LIB_MAN_SUFFIX) .man

.man.$(LIB_MAN_SUFFIX):
	-rm -f $@
	$(LN_S) $< $@
