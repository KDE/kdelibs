#$Id$

INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/kio -I$(top_srcdir)/kfile $(all_includes)

bin_PROGRAMS = cupsdconf
cupsdconf_SOURCES = main.cpp

cupsdconf_LDADD   =  libcupsdconf.la
cupsdconf_LDFLAGS = $(all_libraries) $(KDE_RPATH)

# library creation
lib_LTLIBRARIES = libcupsdconf.la
libcupsdconf_la_SOURCES = cupsdconf.cpp cupsddialog.cpp cupsdpage.cpp cupsdcomment.cpp cupsdsplash.cpp \
			  cupsdserverpage.cpp cupsdlogpage.cpp cupsdjobspage.cpp cupsdfilterpage.cpp \
			  qdirlineedit.cpp cupsddirpage.cpp portdialog.cpp cupsdnetworkpage.cpp \
			  editlist.cpp cupsdbrowsingpage.cpp browsedialog.cpp cupsdsecuritypage.cpp \
			  locationdialog.cpp addressdialog.cpp cups-util.c \
			  qdirmultilineedit.cpp
libcupsdconf_la_METASOURCES = AUTO
libcupsdconf_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
libcupsdconf_la_LIBADD = $(top_builddir)/kio/libkio.la $(LIB_CUPS)

noinst_HEADERS = cupsdconf.h cupsdpage.h cupsddialog.h cupsdcomment.h cupsdsplash.h \
		 cupsdserverpage.h cupsdlogpage.h cupsdjobspage.h cupsdfilterpage.h \
		 qdirlineedit.h cupsddirpage.h portdialog.h cupsdnetworkpage.h \
		 editlist.h cupsdbrowsingpage.h browsedialog.h cupsdsecuritypage.h \
		 locationdialog.h addressdialog.h cups-util.h qdirmultilineedit.h

# Data
cupsdconfdata_DATA = kde_logo.png cups_logo.png cupsd.conf.template
cupsdconfdatadir   = $(kde_datadir)/kdeprint

# Translations
messages:
	$(XGETTEXT) $(libcupsdconf_la_SOURCES) main.cpp -o $(podir)/cupsdconf.pot
