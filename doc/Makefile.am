
SUBDIRS = kdebug kfile kspell kded komop

data_DATA = 

datadir = $(kde_htmldir)/en/kdelibs

EXTRA_DIST =

install-data-local:
	-rm -f $(DESTDIR)$(kde_htmldir)/default
	$(LN_S) en $(DESTDIR)$(kde_htmldir)/default
