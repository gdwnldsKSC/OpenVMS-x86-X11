DISTCHECK_CONFIGURE_FLAGS = --with-cursordir=\$${datadir}/icons

SUBDIRS = handhelds redglass whiteglass

EXTRA_DIST = genmakefile.sh ChangeLog

AUTOMAKE_OPTIONS = foreign

dist-hook: ChangeLog

.PHONY: ChangeLog

ChangeLog:
	$(CHANGELOG_CMD)
