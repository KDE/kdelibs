
SUBDIRS = kfile kspell

data_DATA = kdebug.html

datadir = $(kde_htmldir)/en/kdelibs

EXTRA_DIST = kdebug.html

install-data-local:
	-rm -f $(DESTDIR)(kde_htmldir)/default
	$(LN_S) en $(DESTDIR)$(kde_htmldir)/default
