EXTRA_DIST=$(wildcard *.po) POTFILES POTFILES.in

DISTCLEANFILES=.intltool-merge-cache POTFILES stamp-it

update-pot:
	$(INTLTOOL_UPDATE) -p -g $(PACKAGE)

update-po:
	for i in *.po; do echo Updating $$i; $(INTLTOOL_UPDATE) `basename $$i .po` -g $(PACKAGE); done
