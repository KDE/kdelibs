INCLUDES = $(all_includes) -I$(top_srcdir)/arts/mcop -I$(top_srcdir)/arts/flow \
   -I$(top_builddir)/arts/flow
LDFLAGS   = $(all_libraries) $(top_builddir)/arts/mcop/libmcop.la \
   $(LIBAUDIOFILE)
MCOPIDL = $(top_builddir)/arts/mcopidl/mcopidl
FLOWLIBS = $(LDFLAGS) $(top_builddir)/arts/flow/libartsflow.la

####### Files

bin_PROGRAMS = helloserver helloclient referenceinfo testflow testdynflow \
  irdemo artsd artsplay

helloserver_SOURCES = hello.cc helloserver.cc hello_impl.cc
helloclient_SOURCES = hello.cc helloclient.cc
referenceinfo_SOURCES = referenceinfo.cc
irdemo_SOURCES = irdemo.cc

artsd_LDFLAGS = $(FLOWLIBS)
artsd_SOURCES = soundserver.cc simplesoundserver_impl.cc artsd.cc

artsplay_SOURCES = soundserver.cc artsplay.cc

testflow_LDFLAGS = $(FLOWLIBS)
testflow_SOURCES = testflow.cc

testdynflow_LDFLAGS = $(FLOWLIBS)
testdynflow_SOURCES = testdynflow.cc


DISTCLEANFILES = $(SRCMETA) hello.cc hello.h soundserver.cc soundserver.h

####### Implicit rules

####### Build rules

hello.h: $(top_srcdir)/arts/examples/hello.idl $(MCOPIDL)
	$(MCOPIDL) $(top_srcdir)/arts/examples/hello.idl

hello.cc: hello.h

soundserver.h: $(top_srcdir)/arts/examples/soundserver.idl $(MCOPIDL)
	$(MCOPIDL) $(top_srcdir)/arts/examples/soundserver.idl

soundserver.cc: soundserver.h
