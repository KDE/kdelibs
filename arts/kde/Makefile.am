#SUBDIRS = . mcop-dcop
SUBDIRS = . 
INCLUDES = -I$(top_srcdir)/arts \
		   -I$(top_srcdir)/arts/mcop -I$(top_srcdir)/arts/qtmcop -I$(top_srcdir)/arts/flow \
           -I$(top_builddir)/arts/flow \
           -I$(top_srcdir)/arts/soundserver \
           -I$(top_builddir)/arts/soundserver -I$(top_builddir)/arts/kde/lib \
           -I$(top_srcdir) $(all_includes)
lib_LTLIBRARIES  	    =	libartskde.la
libartskde_la_SOURCES	    =	artskde.cc kioinputstream_impl.cpp kplayobject.cc \
				kplayobjectfactory.cc kartsfloatwatch.cpp kartsdispatcher.cc
libartskde_la_LIBADD        =   $(LIB_KSYCOCA) $(top_builddir)/arts/qtmcop/libqtmcop.la $(top_builddir)/arts/soundserver/libsoundserver_idl.la
libartskde_la_LDFLAGS       =   $(all_libraries) -avoid-version -no-undefined
libartskde_la_METASOURCES   =	AUTO

artskdeincludedir	    =	$(includedir)/arts
artskdeinclude_HEADERS      = 	kplayobject.h kplayobjectfactory.h kartsfloatwatch.h artskde.h kartsdispatcher.h

$(srcdir)/kioinputstream_impl.cpp: artskde.h
$(srcdir)/kartsfloatwatch.cpp: artskde.h
artskde.cc: artskde.h
artskde.mcoptype: artskde.h
artskde.mcopclass: artskde.h
artskde.h: $(top_srcdir)/arts/kde/artskde.idl $(MCOPIDL)
	    $(MCOPIDL) -I$(top_srcdir)/arts/flow -I$(top_srcdir)/arts/soundserver -t $(MCOPINC) $(top_srcdir)/arts/kde/artskde.idl

DISTCLEANFILES = artskde.cc artskde.h artskde.mcoptype artskde.mcopclass

check_PROGRAMS = kiotest kiotestslow
kiotest_SOURCES = kiotest.cc
kiotestslow_SOURCES = kiotestslow.cc

kiotest_LDADD	= libartskde.la -lqt -lqtmcop $(top_builddir)/arts/soundserver/libkmedia2.la $(top_builddir)/arts/soundserver/libsoundserver_idl.la
kiotestslow_LDADD	= $(kiotest_LDADD)
