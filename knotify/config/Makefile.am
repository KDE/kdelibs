

INCLUDES= -I$(srcdir) $(all_includes)

lib_LTLIBRARIES = libknotifyconfig.la

libknotifyconfig_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) -version-info 1:1 -no-undefined
libknotifyconfig_la_LIBADD = $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_DCOP) $(LIB_KFILE)
libknotifyconfig_la_SOURCES =   knotifyconfigactionswidgetbase.ui knotifyconfigactionswidget.cpp knotifyconfigelement.cpp knotifyeventlist.cpp knotifyconfigwidget.cpp

libknotifyconfig_la_METASOURCES = AUTO

include_HEADERS = knotifyconfigwidget.h
