
SUBDIRS = kfile kspell

data_DATA = kdebug.html

datadir = $(kde_htmldir)/en/kdelibs

EXTRA_DIST = kdebug.html

install-data-local:
	$(mkinstalldirs) $(kde_htmldir)
	-rm -f $(kde_htmldir)/default
	$(LN_S) en $(kde_htmldir)/default

