
INCLUDES = $(all_includes) -I$(top_srcdir)/arts/mcop -I$(top_srcdir)/arts/flow \
   -I$(top_builddir)/arts/flow
MCOPINC   = -I$(srcdir) -I$(top_srcdir)/arts/flow
MCOPIDL   = $(top_builddir)/arts/mcopidl/mcopidl
FLOWLIBS  = $(top_builddir)/arts/flow/libartsflow.la

####### install idl files

artsincludedir = $(includedir)/arts
artsinclude_HEADERS = soundserver.h soundserver.idl kmedia2.h kmedia2.idl

####### libsoundserver_idl, libkmedia2_idl (idl files as library)

lib_LTLIBRARIES = libsoundserver_idl.la libkmedia2_idl.la

libsoundserver_idl_la_LIBADD = libkmedia2_idl.la \
                          $(top_builddir)/arts/flow/libartsflow_idl.la
libsoundserver_idl_la_LDFLAGS = -no-undefined
libsoundserver_idl_la_SOURCES = soundserver.cc
libkmedia2_idl_la_SOURCES = kmedia2.cc
libkmedia2_idl_la_LIBADD  = $(top_builddir)/arts/mcop/libmcop.la
libkmedia2_idl_la_LDFLAGS = -no-undefined

###### "real" programs

bin_PROGRAMS = artsd artsplay artscat artswrapper

artsd_LDADD = libsoundserver_idl.la $(FLOWLIBS)
artsd_SOURCES = simplesoundserver_impl.cc artsd.cc wavplayobject_impl.cc \
				cpuusage.cc

artscat_LDADD = libsoundserver_idl.la $(FLOWLIBS)
artscat_SOURCES = artscat.cc

artsplay_LDADD = libsoundserver_idl.la
artsplay_SOURCES = artsplay.cc

artswrapper_OBJS = artswrapper.o

bd = `echo '$(bindir)' |sed -e "s,\\",,g"`

artswrapper.o: $(srcdir)/artswrapper.c
	$(CC) -c $(DEFS) $(INCLUDES) -DEXECUTE=\"$(bd)/artsd\" \
			$(srcdir)/artswrapper.c

DISTCLEANFILES = soundserver.cc soundserver.h kmedia2.h kmedia2.cc

####### Build rules

soundserver.h: $(top_srcdir)/arts/soundserver/soundserver.idl kmedia2.h $(MCOPIDL)
	$(MCOPIDL) $(MCOPINC) $(top_srcdir)/arts/soundserver/soundserver.idl

soundserver.cc: soundserver.h
artscat.cc: soundserver.h
artsd.cc: soundserver.h
artsplay.cc: soundserver.h
wavplayobject_impl.cc: soundserver.h
simplesoundserver_impl.cc: soundserver.h

kmedia2.h: $(top_srcdir)/arts/soundserver/kmedia2.idl $(MCOPIDL)
	$(MCOPIDL) $(top_srcdir)/arts/soundserver/kmedia2.idl

kmedia2.cc: kmedia2.h

mcopclassdir = $(libdir)/Arts
if HAVE_WAVSUPPORT
mcopclass_DATA = WavPlayObject.mcopclass
endif

