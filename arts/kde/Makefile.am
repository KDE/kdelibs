INCLUDES = -I$(top_builddir)/arts/kde \
           -I$(top_srcdir) -I$(includedir)/arts $(all_includes)

lib_LTLIBRARIES = libartskde.la
libartskde_la_SOURCES = artskde.cc kioinputstream_impl.cpp kplayobject.cc \
                        kplayobjectfactory.cc kartsfloatwatch.cpp kartsdispatcher.cc \
                        kaudiorecordstream.cpp kaudioplaystream.cpp \
                        kartsserver.cpp kdatarequest_impl.cpp kaudioconverter.cc \
                        kvideowidget.cpp kplayobjectcreator.cc \
                        kaudiomanagerplay.cpp
libartskde_la_LIBADD = $(LIB_KIO) -lqtmcop -lsoundserver_idl
libartskde_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 3:0:2
libartskde_la_METASOURCES = AUTO
libartskde_la_COMPILE_FIRST = artskde.h

artskdeincludedir = $(includedir)/arts
artskdeinclude_HEADERS = kplayobject.h kplayobjectfactory.h kartsfloatwatch.h \
                         artskde.h kartsdispatcher.h \
                         kaudiorecordstream.h kaudioplaystream.h \
                         kartsserver.h kvideowidget.h kaudiomanagerplay.h
noinst_HEADERS = kplayobjectcreator.h kplayobjectfactory_p.h

artskde.mcoptype: artskde.h
artskde.mcopclass: artskde.h
MCOPINC = -I$(srcdir)
artskde.cc artskde.h: $(top_srcdir)/arts/kde/artskde.idl $(MCOPIDL)
	     $(MCOPIDL) -I$(includedir)/arts -t $(MCOPINC) $(top_srcdir)/arts/kde/artskde.idl

DISTCLEANFILES = artskde.cc artskde.h artskde.mcoptype artskde.mcopclass

check_PROGRAMS = kiotest kiotestslow kconverttest
kiotest_SOURCES = kiotest.cc
kiotestslow_SOURCES = kiotestslow.cc
kconverttest_SOURCES = kconverttest.cc

kiotest_LDADD = libartskde.la -lqtmcop -lkmedia2 -lsoundserver_idl
kiotestslow_LDADD = $(kiotest_LDADD)
kconverttest_LDADD = $(kiotest_LDADD)

# vim: ts=8 noet
