
INCLUDES = $(all_includes) -I$(top_srcdir)/arts/mcop -I$(top_srcdir)/arts/flow \
   -I$(top_builddir)/arts/flow
LDFLAGS   = $(all_libraries) $(top_builddir)/arts/mcop/libmcop.la
MCOPIDL   = $(top_builddir)/arts/mcopidl/mcopidl
FLOWLIBS  = $(top_builddir)/arts/flow/libartsflow.la

####### libsoundserver_idl (idl file as library)

lib_LTLIBRARIES = libsoundserver_idl.la

libsoundserver_idl_la_SOURCES = soundserver.cc

artsincludedir = $(includedir)/arts
artsinclude_HEADERS = soundserver.h soundserver.idl

#
# -frepo closure (experimental)
#
closure: $(libsoundserver_idl_la_OBJECTS)
	@echo "Building -frepo compliant library closure"
	@echo "int main() {}" > closure.cc
	$(CXXLINK) -o closure $(libsoundserver_idl_la_OBJECTS) closure.cc
	@echo "closure timestamp" > closure
	@rm -f closure.cc

###### artsd, artsplay

bin_PROGRAMS = artsd artsplay

artsd_LDADD = libsoundserver_idl.la $(FLOWLIBS) $(LIBAUDIOFILE)
artsd_SOURCES = simplesoundserver_impl.cc artsd.cc

artsplay_LDADD = libsoundserver_idl.la
artsplay_SOURCES = artsplay.cc

DISTCLEANFILES = soundserver.cc soundserver.h

####### Build rules

soundserver.h: $(top_srcdir)/arts/soundserver/soundserver.idl $(MCOPIDL)
	$(MCOPIDL) $(top_srcdir)/arts/soundserver/soundserver.idl

soundserver.cc: soundserver.h
