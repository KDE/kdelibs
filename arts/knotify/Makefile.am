
INCLUDES= -I$(top_srcdir)/arts/mcop -I$(top_srcdir)/arts/qtmcop \
	-I$(top_builddir)/arts/flow \
	-I$(top_srcdir)/arts/soundserver -I$(top_builddir)/arts/soundserver \
	$(all_includes)

####### Files

bin_PROGRAMS	= knotify
knotify_SOURCES	= knotify.cpp knotify.skel
knotify_LDADD	= ../soundserver/libsoundserver_idl.la \
                  ../qtmcop/libqtmcop.la $(LIB_KDEUI) $(LIB_KDECORE)
knotify_LDFLAGS	= $(all_libraries) $(KDE_RPATH)
METASOURCES = AUTO

check_PROGRAMS		= knotifytest
knotifytest_SOURCES	= knotifytest.cpp
knotifytest_LDADD	= $(LIB_KDECORE)
knotifytest_LDFLAGS	= $(all_libraries) $(KDE_RPATH)

noinst_HEADERS	=  knotify.h

messages:
	$(XGETTEXT) $(knotify_SOURCES) -o $(podir)/knotify.pot

