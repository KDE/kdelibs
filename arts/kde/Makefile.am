INCLUDES                    =   $(all_includes) -I$(top_srcdir)/arts \
			        -I$(top_srcdir)/arts/mcop -I$(top_srcdir)/arts/qtmcop -I$(top_srcdir)/arts/flow \
				-I$(top_builddir)/arts/flow \
				-I$(top_srcdir)/arts/soundserver \
				-I$(top_builddir)/arts/soundserver -I$(top_builddir)/arts/kde/lib \
				-I$(top_srcdir)
lib_LTLIBRARIES  	    =	libartskde.la
libartskde_la_SOURCES	    =	artskde.cc kioinputstream_impl.cpp kplayobject.cc \
				kplayobjectfactory.cc kartsfloatwatch.cpp
libartskde_la_LIBADD        =   $(LIB_KDECORE) $(LIB_KIO) $(LIB_KSYCOCA) $(LIB_QT) -L../qtmcop -lqtmcop
libartskde_la_LDFLAGS       =   -avoid-version -module $(all_libraries)
libartskde_la_METASOURCES   =	AUTO

artskdeincludedir	    =	$(includedir)/arts
artskdeinclude_HEADERS      = 	kplayobject.h kplayobjectfactory.h kartsfloatwatch.h

$(srcdir)/kioinputstream_impl.cpp: artskde.h
$(srcdir)/kartsfloatwatch.cpp: artskde.h
artskde.cc: artskde.h
artskde.mcoptype: artskde.h
artskde.mcopclass: artskde.h
artskde.h: $(top_srcdir)/arts/kde/artskde.idl $(MCOPIDL)
	    $(MCOPIDL) -I$(top_srcdir)/arts/flow -I$(top_srcdir)/arts/soundserver -t $(MCOPINC) $(top_srcdir)/arts/kde/artskde.idl

check_PROGRAMS = kiotest kiotestslow
kiotest_SOURCES = kiotest.cc
kiotestslow_SOURCES = kiotestslow.cc

kiotest_LDADD	= libartskde.la -lqt -lqtmcop $(top_builddir)/arts/soundserver/libkmedia2.la $(top_builddir)/arts/soundserver/libsoundserver_idl.la
kiotestslow_LDADD	= $(kiotest_LDADD)
