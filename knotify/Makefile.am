
INCLUDES= $(all_includes) $(akode_includes)
KDE_CXXFLAGS = -UQT3_SUPPORT
####### Files

#kde_module_LTLIBRARIES = knotify.la
lib_LTLIBRARIES =
kdeinit_LTLIBRARIES = knotify.la

knotify_la_SOURCES	= knotify.cpp knotify.skel
knotify_la_LIBADD	= $(LIB_KDEUI) $(akode_libs)
knotify_la_LDFLAGS = $(all_libraries) -module -avoid-version
knotify_la_METASOURCES = AUTO

check_PROGRAMS		= knotifytest
knotifytest_SOURCES	= knotifytest.cpp
knotifytest_LDADD	= $(LIB_KDECORE)
knotifytest_LDFLAGS	= $(all_libraries) $(KDE_RPATH)
knotifytest_CFLAGS 	= -I../kdecore

noinst_HEADERS	=  knotify.h

kdelnkdir = $(kde_servicesdir)
kdelnk_DATA = knotify.desktop

messages:
	$(XGETTEXT) knotify.cpp -o $(podir)/knotify.pot

