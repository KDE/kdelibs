#$Id: Makefile.am 285549 2004-02-06 13:39:51Z dfaure $

INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/kio -I$(top_srcdir)/kfile $(all_includes)

bin_PROGRAMS =
lib_LTLIBRARIES =
kdeinit_LTLIBRARIES = cupsdconf.la

# library creation
cupsdconf_la_SOURCES = cupsdconf.cpp cupsddialog.cpp cupsdpage.cpp cupsdcomment.cpp cupsdsplash.cpp \
			  cupsdserverpage.cpp cupsdlogpage.cpp cupsdjobspage.cpp cupsdfilterpage.cpp \
			  qdirlineedit.cpp cupsddirpage.cpp portdialog.cpp cupsdnetworkpage.cpp \
			  editlist.cpp cupsdbrowsingpage.cpp browsedialog.cpp cupsdsecuritypage.cpp \
			  locationdialog.cpp addressdialog.cpp cups-util.c \
			  qdirmultilineedit.cpp sizewidget.cpp main.cpp
cupsdconf_la_METASOURCES = AUTO
cupsdconf_la_LDFLAGS = -avoid-version -module $(all_libraries) $(KDE_RPATH)
cupsdconf_la_LIBADD = $(LIB_KIO) $(LIB_CUPS)

noinst_HEADERS = cupsdconf.h cupsdpage.h cupsddialog.h cupsdcomment.h cupsdsplash.h \
		 cupsdserverpage.h cupsdlogpage.h cupsdjobspage.h cupsdfilterpage.h \
		 qdirlineedit.h cupsddirpage.h portdialog.h cupsdnetworkpage.h \
		 editlist.h cupsdbrowsingpage.h browsedialog.h cupsdsecuritypage.h \
		 locationdialog.h addressdialog.h cups-util.h qdirmultilineedit.h \
		 sizewidget.h

# Data
cupsdconfdata_DATA = kde_logo.png cups_logo.png cupsd.conf.template
cupsdconfdatadir   = $(kde_datadir)/kdeprint

# Translations
messages:
	perl cupsdcomment.pl < cupsd.conf.template > cupsd.conf.template.cpp
	$(XGETTEXT) *.cpp -o $(podir)/cupsdconf.pot
	rm -f cupsd.conf.template.cpp
