INCLUDES = $(all_includes) -I$(top_srcdir)/arts/mcop -I$(top_srcdir)/arts/flow \
   -I$(top_builddir)/arts/flow -I$(top_srcdir)/arts/soundserver \
   -I$(top_builddir)/arts/soundserver -I$(top_srcdir)/arts/artsc

LDFLAGS   = $(all_libraries) $(top_builddir)/arts/mcop/libmcop.la
MCOPIDL = $(top_builddir)/arts/mcopidl/mcopidl
FLOWLIBS = $(LDFLAGS) $(top_builddir)/arts/flow/libartsflow.la

####### Files

check_PROGRAMS = helloserver helloclient referenceinfo \
  irdemo x11commtest streamsound hellomain flow testwrapper artsctest \
  dcasttest
helloserver_SOURCES = hello.cc helloserver.cc hello_impl.cc
helloclient_SOURCES = hello.cc helloclient.cc
referenceinfo_SOURCES = referenceinfo.cc
irdemo_SOURCES = irdemo.cc
x11commtest_SOURCES = x11commtest.cc
hellomain_SOURCES = hellomain.cc hello.cc hello_impl.cc

testwrapper_LDFLAGS = $(FLOWLIBS)
testwrapper_SOURCES = wrapper.cc testwrapper.cc

flow_LDFLAGS = $(FLOWLIBS)
flow_SOURCES = flow.cc

streamsound_LDFLAGS = $(FLOWLIBS) \
    $(top_builddir)/arts/soundserver/libsoundserver_idl.la
streamsound_SOURCES = streamsound.cc

dcasttest_LDFLAGS = $(FLOWLIBS) \
    $(top_builddir)/arts/soundserver/libsoundserver_idl.la
dcasttest_SOURCES = dcasttest.cc

artsctest_LDFLAGS = $(top_builddir)/arts/artsc/libartsc.la
artsctest_SOURCES = artsctest.c

DISTCLEANFILES = $(SRCMETA) hello.cc hello.h

####### Implicit rules

####### Build rules

hello.h: $(top_srcdir)/arts/examples/hello.idl $(MCOPIDL)
	$(MCOPIDL) $(top_srcdir)/arts/examples/hello.idl

hello.cc: hello.h

wrapper.h: $(top_srcdir)/arts/examples/wrapper.idl $(MCOPIDL)
	$(MCOPIDL) $(top_srcdir)/arts/examples/wrapper.idl

wrapper.cc: wrapper.h
