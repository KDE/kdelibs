
INCLUDES = $(all_includes) -I$(top_srcdir)/arts/mcop -I$(top_srcdir)/arts/flow \
   -I$(top_builddir)/arts/flow
MCOPINC   = -I$(srcdir) -I$(top_srcdir)/arts/flow
LDFLAGS   = $(all_libraries) $(top_builddir)/arts/mcop/libmcop.la
MCOPIDL   = $(top_builddir)/arts/mcopidl/mcopidl
FLOWLIBS  = $(top_builddir)/arts/flow/libartsflow.la

####### install idl files

artsincludedir = $(includedir)/arts
artsinclude_HEADERS = soundserver.h soundserver.idl kmedia2.h kmedia2.idl

####### libsoundserver_idl, libkmedia2_idl (idl files as library)

lib_LTLIBRARIES = libsoundserver_idl.la libkmedia2_idl.la

libsoundserver_idl_la_LIBADD = libkmedia2_idl.la \
                          $(top_builddir)/arts/flow/libartsflow_idl.la
libsoundserver_idl_la_SOURCES = soundserver.cc
libkmedia2_idl_la_SOURCES = kmedia2.cc

###### -frepo closure (experimental)

EXTRA_PROGRAMS = closure_kmedia2_idl closure_soundserver_idl

closure_main.cc:
	@echo "int main() {}" > closure_main.cc

closure_kmedia2_idl_SOURCES = closure_main.cc
closure_kmedia2_idl_LDADD = $(libkmedia2_idl_la_OBJECTS)

closure_soundserver_idl_SOURCES = closure_main.cc
closure_soundserver_idl_LDADD = $(libsoundserver_idl_la_OBJECTS) \
                                $(libsoundserver_idl_la_LIBADD)

closure: closure_kmedia2_idl closure_soundserver_idl

###### "real" programs

bin_PROGRAMS = artsd artsplay

artsd_LDADD = libsoundserver_idl.la $(FLOWLIBS) $(LIBAUDIOFILE)
artsd_SOURCES = simplesoundserver_impl.cc artsd.cc wavplayobject_impl.cc

artsplay_LDADD = libsoundserver_idl.la
artsplay_SOURCES = artsplay.cc

DISTCLEANFILES = soundserver.cc soundserver.h kmedia2.h kmedia2.cc

####### Build rules

soundserver.h: $(top_srcdir)/arts/soundserver/soundserver.idl kmedia2.h $(MCOPIDL)
	$(MCOPIDL) $(MCOPINC) $(top_srcdir)/arts/soundserver/soundserver.idl

soundserver.cc: soundserver.h

kmedia2.h: $(top_srcdir)/arts/soundserver/kmedia2.idl $(MCOPIDL)
	$(MCOPIDL) $(top_srcdir)/arts/soundserver/kmedia2.idl

kmedia2.cc: kmedia2.h
