# $Id$

# necessary for artswrapper
artsdpath = $(bindir)/artsd

INCLUDES = -I$(top_srcdir)/arts/mcop -I$(top_srcdir)/arts/flow \
   -I$(top_builddir)/arts/flow -DEXECUTE=\"$(artsdpath)\" $(all_includes)
MCOPINC   = -I$(srcdir) -I$(top_srcdir)/arts/flow
FLOWLIBS  = $(top_builddir)/arts/flow/libartsflow.la

####### install idl files

artsincludedir = $(includedir)/arts
artsinclude_HEADERS = soundserver.h soundserver.idl kmedia2.h kmedia2.idl \
	artsversion.h

####### libsoundserver_idl, libkmedia2_idl (idl files as library)

lib_LTLIBRARIES = libkmedia2_idl.la libsoundserver_idl.la \
						libartswavplayobject.la libkmedia2.la

libsoundserver_idl_la_LIBADD = libkmedia2_idl.la \
                          $(top_builddir)/arts/flow/libartsflow_idl.la
libsoundserver_idl_la_LDFLAGS = -no-undefined -version-info 1:0
libsoundserver_idl_la_SOURCES = soundserver.cc

libkmedia2_idl_la_SOURCES = kmedia2.cc
libkmedia2_idl_la_LIBADD  = $(top_builddir)/arts/flow/libartsflow.la
libkmedia2_idl_la_LDFLAGS = -no-undefined -version-info 1:0

libkmedia2_la_SOURCES = fileinputstream_impl.cc stdoutwriter_impl.cc
libkmedia2_la_LIBADD  = libkmedia2_idl.la $(FLOWLIBS)
libkmedia2_la_LDFLAGS = -no-undefined -version-info 1:0

libartswavplayobject_la_SOURCES = wavplayobject_impl.cc
libartswavplayobject_la_LIBADD  = $(top_builddir)/arts/mcop/libmcop.la \
                                  libsoundserver_idl.la $(FLOWLIBS)
libartswavplayobject_la_LDFLAGS = -no-undefined -module


###### "real" programs

bin_PROGRAMS = artsd artsplay artscat artswrapper artsshell artsrec

artsd_LDADD = libsoundserver_idl.la $(FLOWLIBS) \
              $(top_builddir)/arts/mcop_mt/libmcop_mt.la $(USE_THREADS) $(LIBPTHREAD)
artsd_SOURCES = soundserverv2_impl.cc soundserver_impl.cc simplesoundserver_impl.cc artsd.cc cpuusage.cc samplestorage_impl.cc mcopinfo_impl.cc

artscat_LDADD = libsoundserver_idl.la $(FLOWLIBS)
artscat_SOURCES = artscat.cc

artsrec_LDADD = libsoundserver_idl.la $(FLOWLIBS)
artsrec_SOURCES = artsrec.cc

artsplay_LDADD = libsoundserver_idl.la 
artsplay_SOURCES = artsplay.cc

artswrapper_SOURCES = artswrapper.c
artswrapper_INCLUDES = -I$(top_builddir) -DEXECUTE=\"$(artsdpath)\"

artsshell_LDADD = libsoundserver_idl.la $(FLOWLIBS)
artsshell_SOURCES = artsshell.cc

DISTCLEANFILES = soundserver.cc soundserver.h soundserver.mcopclass \
  soundserver.mcoptype kmedia2.h kmedia2.cc kmedia2.mcopclass kmedia2.mcoptype

####### Build rules

samplestorage_impl.cc: soundserver.h
soundserver.cc: soundserver.h
soundserver.mcoptype: soundserver.h
soundserver.mcopclass: soundserver.h
soundserver.h: $(top_srcdir)/arts/soundserver/soundserver.idl kmedia2.h $(MCOPIDL)
	$(MCOPIDL) -t $(MCOPINC) $(top_srcdir)/arts/soundserver/soundserver.idl


kmedia2.cc: kmedia2.h soundserver.h
kmedia2.mcoptype: kmedia2.h
kmedia2.mcopclass: kmedia2.h
kmedia2.h: $(top_srcdir)/arts/soundserver/kmedia2.idl $(MCOPIDL)
	$(MCOPIDL) -t $(MCOPINC) $(top_srcdir)/arts/soundserver/kmedia2.idl


artsversion.h: artsversion-new.h
	(cmp -s artsversion-new.h artsversion.h \
	  || cp artsversion-new.h artsversion.h) 

artscat.cc: soundserver.h artsversion.h
artsrec.cc: soundserver.h artsversion.h
artsd.cc: soundserver.h artsversion.h
artsplay.cc: soundserver.h artsversion.h
artsshell.cc: soundserver.h artsversion.h
wavplayobject_impl.cc: soundserver.h artsversion.h
simplesoundserver_impl.cc: soundserver.h artsversion.h
soundserver_impl.cc: soundserver.h artsversion.h
soundserverv2_impl.cc: soundserver.h artsversion.h
samplestorage_impl.cc: soundserver.h artsversion.h

mcopclass_UNCONDITIONAL = FileInputStream.mcopclass

mcopclassdir = $(libdir)/mcop/Arts
if HAVE_WAVSUPPORT
mcopclass_DATA = WavPlayObject.mcopclass $(mcopclass_UNCONDITIONAL)
else
mcopclass_DATA = $(mcopclass_UNCONDITIONAL)
endif

install-exec-local: install-binPROGRAMS
	@(chown root $(bindir)/artswrapper && chmod 4755 $(bindir)/artswrapper) \
		|| echo "please make $(bindir)/artswrapper suid root"

######## install idl typeinfo files

mcoptypedir = $(libdir)/mcop
mcoptype_DATA = soundserver.mcoptype soundserver.mcopclass \
	kmedia2.mcoptype kmedia2.mcopclass
