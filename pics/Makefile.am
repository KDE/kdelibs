SUBDIRS = . hicolor crystalsvg emoticons

INCLUDES = $(LIBART_CFLAGS) -I$(top_srcdir)/kdecore/svgicons -I$(top_srcdir)/kdefx $(all_includes)

if include_SVGICONS
KSVGTOPNG=ksvgtopng
endif

bin_PROGRAMS = $(KSVGTOPNG)

ksvgtopng_SOURCES = ksvgtopng.cpp
ksvgtopng_LDADD =  $(LIBART_LIBS) $(LIB_KDECORE) $(top_builddir)/kdecore/svgicons/libkdesvgicons.la
ksvgtopng_LDFLAGS = $(KDE_RPATH) $(all_libraries)

install-data-local:
	-rm -f $(DESTDIR)$(kde_icondir)/default.kde
	$(mkinstalldirs) $(DESTDIR)$(kde_icondir)
	$(LN_S) crystalsvg $(DESTDIR)$(kde_icondir)/default.kde
