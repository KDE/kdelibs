SUBDIRS = . mcop-dcop
INCLUDES = -I$(includedir)/arts \
           -I$(top_builddir)/arts/kde/lib \
           -I$(top_srcdir) $(all_includes)
lib_LTLIBRARIES  	    =	libartskde.la
libartskde_la_SOURCES	    =	artskde.cc kioinputstream_impl.cpp kplayobject.cc \
				kplayobjectfactory.cc kartsfloatwatch.cpp kartsdispatcher.cc \
				kreceiver.cpp kartsserver.cpp
libartskde_la_LIBADD        =   $(LIB_KSYCOCA) -lqtmcop -lsoundserver_idl
libartskde_la_LDFLAGS       =   $(all_libraries) -no-undefined -version-info 1:0
libartskde_la_METASOURCES   =	AUTO

artskdeincludedir	    =	$(includedir)/arts
artskdeinclude_HEADERS      = 	kplayobject.h kplayobjectfactory.h kartsfloatwatch.h artskde.h kartsdispatcher.h kreceiver.h kartsserver.h

$(srcdir)/kioinputstream_impl.cpp: artskde.h
$(srcdir)/kartsfloatwatch.cpp: artskde.h
artskde.cc: artskde.h
artskde.mcoptype: artskde.h
artskde.mcopclass: artskde.h
artskde.h: $(top_srcdir)/arts/kde/artskde.idl $(MCOPIDL)
	    $(MCOPIDL) -I$(includedir)/arts -I$(includedir)/arts -t $(MCOPINC) $(top_srcdir)/arts/kde/artskde.idl

DISTCLEANFILES = artskde.cc artskde.h artskde.mcoptype artskde.mcopclass

check_PROGRAMS = kiotest kiotestslow
kiotest_SOURCES = kiotest.cc
kiotestslow_SOURCES = kiotestslow.cc

kiotest_LDADD	= libartskde.la -lqtmcop -lkmedia2 -lsoundserver_idl
kiotestslow_LDADD	= $(kiotest_LDADD)
