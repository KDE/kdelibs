
INCLUDES= -I$(includedir)/arts $(all_includes)

####### Files

bin_PROGRAMS	= knotify
lib_LTLIBRARIES = kdeinit_knotify.la

kdeinit_knotify_la_SOURCES	= knotify.cpp knotify.skel
kdeinit_knotify_la_LIBADD	= -lsoundserver_idl -lqtmcop $(LIB_KDEUI)
kdeinit_knotify_la_LDFLAGS = $(all_libraries) -module -avoid-version
kdeinit_knotify_la_METASOURCES = AUTO

knotify_SOURCES = dummy.cpp
knotify_LDADD = kdeinit_knotify.la
knotify_LDFLAGS  = $(all_libraries) $(KDE_RPATH)

check_PROGRAMS		= knotifytest
knotifytest_SOURCES	= knotifytest.cpp
knotifytest_LDADD	= $(LIB_KDECORE)
knotifytest_LDFLAGS	= $(all_libraries) $(KDE_RPATH)

noinst_HEADERS	=  knotify.h

kdelnkdir = $(kde_servicesdir)
kdelnk_DATA = knotify.desktop

messages:
	$(XGETTEXT) knotify.cpp -o $(podir)/knotify.pot

dummy.cpp:
	echo > dummy.cpp
