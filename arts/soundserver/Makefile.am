
INCLUDES = $(all_includes) -I$(top_srcdir)/arts/mcop -I$(top_srcdir)/arts/flow \
   -I$(top_builddir)/arts/flow
MCOPINC   = -I$(srcdir) -I$(top_srcdir)/arts/flow
FLOWLIBS  = $(top_builddir)/arts/flow/libartsflow.la

####### install idl files

artsincludedir = $(includedir)/arts
artsinclude_HEADERS = soundserver.h soundserver.idl kmedia2.h kmedia2.idl

####### libsoundserver_idl, libkmedia2_idl (idl files as library)

lib_LTLIBRARIES = libkmedia2_idl.la libsoundserver_idl.la \
						libartswavplayobject.la

libsoundserver_idl_la_LIBADD = libkmedia2_idl.la \
                          $(top_builddir)/arts/flow/libartsflow_idl.la
libsoundserver_idl_la_LDFLAGS = -no-undefined
libsoundserver_idl_la_SOURCES = soundserver.cc
libkmedia2_idl_la_SOURCES = kmedia2.cc
libkmedia2_idl_la_LIBADD  = $(top_builddir)/arts/mcop/libmcop.la
libkmedia2_idl_la_LDFLAGS = -no-undefined

libartswavplayobject_la_SOURCES = wavplayobject_impl.cc
libartswavplayobject_la_LIBADD  = $(top_builddir)/arts/mcop/libmcop.la \
								  libsoundserver_idl.la $(FLOWLIBS)
libartswavplayobject_la_LDFLAGS = -no-undefined -module


###### "real" programs

bin_PROGRAMS = artsd artsplay artscat artswrapper artsshell

artsd_LDADD = libsoundserver_idl.la $(FLOWLIBS) \
              $(top_builddir)/arts/mcop_mt/libmcop_mt.la
artsd_SOURCES = soundserverv2_impl.cc soundserver_impl.cc simplesoundserver_impl.cc artsd.cc cpuusage.cc

artscat_LDADD = libsoundserver_idl.la $(FLOWLIBS)
artscat_SOURCES = artscat.cc

artsplay_LDADD = libsoundserver_idl.la
artsplay_SOURCES = artsplay.cc

artswrapper_OBJS = artswrapper.o

artsshell_LDADD = libsoundserver_idl.la $(FLOWLIBS)
artsshell_SOURCES = artsshell.cc

DISTCLEANFILES = soundserver.cc soundserver.h kmedia2.h kmedia2.cc

####### Build rules

soundserver.h: $(top_srcdir)/arts/soundserver/soundserver.idl kmedia2.h $(MCOPIDL)
	$(MCOPIDL) -t $(MCOPINC) $(top_srcdir)/arts/soundserver/soundserver.idl

soundserver.cc: soundserver.h

kmedia2.h: $(top_srcdir)/arts/soundserver/kmedia2.idl $(MCOPIDL)
	$(MCOPIDL) -t $(top_srcdir)/arts/soundserver/kmedia2.idl

kmedia2.cc: kmedia2.h

artscat.cc: soundserver.h
artsd.cc: soundserver.h
artsplay.cc: soundserver.h
artsshell.cc: soundserver.h
wavplayobject_impl.cc: soundserver.h
simplesoundserver_impl.cc: soundserver.h
soundserver_impl.cc: soundserver.h
soundserverv2_impl.cc: soundserver.h

bd = `echo '$(bindir)' |sed -e "s,\\",,g"`

artswrapper.o: $(srcdir)/artswrapper.c
	$(CC) -c $(DEFS) $(INCLUDES) -DEXECUTE=\"$(bd)/artsd\" \
			$(srcdir)/artswrapper.c


mcopclassdir = $(libdir)/mcop/Arts
if HAVE_WAVSUPPORT
mcopclass_DATA = WavPlayObject.mcopclass
endif

install-exec-local: install-binPROGRAMS
	@(chown root $(bindir)/artswrapper && chmod 4755 $(bindir)/artswrapper) \
		|| echo "please make $(bindir)/artswrapper suid root"

######## install idl typeinfo files

mcoptypedir = $(libdir)/mcop
mcoptype_DATA = soundserver.mcoptype soundserver.mcopclass \
	kmedia2.mcoptype kmedia2.mcopclass
