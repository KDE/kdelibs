#$Id$

INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/kio -I$(top_srcdir)/kfile $(all_includes)

bin_PROGRAMS = cupsdconf
cupsdconf_SOURCES = main.cpp

cupsdconf_LDADD   =  libcupsdconf.la
cupsdconf_LDFLAGS = $(all_libraries) $(KDE_RPATH)

# library creation
lib_LTLIBRARIES = libcupsdconf.la
libcupsdconf_la_SOURCES = cupsdconf.cpp cupsddialog.cpp cupsdoption.cpp cupsdpage.cpp cupsdserverdirpage.cpp \
	  cupsdserverhttppage.cpp cupsdserveridentitypage.cpp cupsdserverlogpage.cpp cupsdservermiscpage.cpp \
	  qdirlineedit.cpp cupsdbrowsinggeneralpage.cpp cupsdbrowsingtimeoutpage.cpp \
	  qinputbox.cpp cupslist.cpp cupsdsplash.cpp cupsdbrowsingmaskspage.cpp cupsdbrowsingconnpage.cpp \
	  cupsdnetworkgeneralpage.cpp cupsrelay.cpp cupsdbrowsingrelaypage.cpp cupsdnetworkclientspage.cpp \
	  cupsdserversecuritypage.cpp cupslocationdialog.cpp cupslocationgeneral.cpp cupslocationaccess.cpp \
	  cupsdcomment.cpp mydialogbase.cpp cupsdserverencryptpage.cpp cupsdserverjobpage.cpp
libcupsdconf_la_METASOURCES = AUTO
libcupsdconf_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
libcupsdconf_la_LIBADD = $(LIB_KIO) $(LIB_CUPS)

noinst_HEADERS = cupsdconf.h cupsdoption.h cupsdpage.h cupsdserverdirpage.h cupsdserverhttppage.h \
	  cupsdserveridentitypage.h cupsdserverlogpage.h cupsdservermiscpage.h qdirlineedit.h \
	  cupsdbrowsinggeneralpage.h cupsdbrowsingtimeoutpage.h qinputbox.h cupslist.h cupsdsplash.h \
	  cupsdbrowsingmaskspage.h cupsdbrowsingconnpage.h cupsdnetworkgeneralpage.h cupsrelay.h \
	  cupsdbrowsingrelaypage.h cupsdnetworkclientspage.h cupsdserversecuritypage.h cupslocationdialog.h \
	  cupslocationgeneral.h cupslocationaccess.h cupsdcomment.h mydialogbase.h cupsdserverencryptpage.h \
	  cupsddialog.h cupsdserverjobpage.h

# Data
cupsdconfdata_DATA = kde_logo.png cups_logo.png cupsd.conf.template
cupsdconfdatadir   = $(kde_datadir)/kdeprint
