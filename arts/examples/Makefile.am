INCLUDES = $(all_includes) -I$(top_srcdir)/arts/mcop -I$(top_srcdir)/arts/flow \
   -I$(top_builddir)/arts/flow -I$(top_srcdir)/arts/soundserver \
   -I$(top_builddir)/arts/soundserver -I$(top_srcdir)/arts/artsc

LDFLAGS   = $(all_libraries) $(top_builddir)/arts/mcop/libmcop.la
FLOWLIBS = $(LDFLAGS) $(top_builddir)/arts/flow/libartsflow.la

####### Files

check_PROGRAMS = helloserver helloclient referenceinfo \
  irdemo x11commtest streamsound hellomain flow artsctest \
  dcasttest hellodynamic testasubsys testaggregation \
  playtofile tradercheck
helloserver_SOURCES = hello.cc helloserver.cc hello_impl.cc
helloclient_SOURCES = hello.cc helloclient.cc
hellodynamic_SOURCES = hello.cc hellodynamic.cc hello_impl.cc
referenceinfo_SOURCES = referenceinfo.cc
tradercheck_SOURCES = tradercheck.cc
irdemo_SOURCES = irdemo.cc
x11commtest_SOURCES = x11commtest.cc
hellomain_SOURCES = hellomain.cc hello.cc hello_impl.cc
testasubsys_SOURCES = testasubsys.cc
testasubsys_LDFLAGS = $(FLOWLIBS)

testaggregation_SOURCES = aggregation.cc testaggregation.cc

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

playtofile_SOURCES = playtofile.cc playtofile_impl.cc playtofile_main.cc

playtofile_LDADD	= $(FLOWLIBS) \
    $(top_builddir)/arts/soundserver/libsoundserver_idl.la \
    $(top_builddir)/arts/soundserver/libkmedia2_idl.la

BUILT_SOURCES = hello.cc hello.h aggregation.cc aggregation.h \
				playtofile.cc playtofile.h
DISTCLEANFILES = $(BUILT_SOURCES)

####### Implicit rules

####### Build rules

hello.h: $(top_srcdir)/arts/examples/hello.idl $(MCOPIDL)
	$(MCOPIDL) $(top_srcdir)/arts/examples/hello.idl

hello.cc: hello.h

aggregation.h: $(top_srcdir)/arts/examples/aggregation.idl $(MCOPIDL)
	$(MCOPIDL) $(top_srcdir)/arts/examples/aggregation.idl

aggregation.cc: aggregation.h

playtofile.cc: playtofile.h

playtofile.h: $(srcdir)/playtofile.idl $(MCOPIDL)
	$(MCOPIDL) -I$(includedir)/arts $(srcdir)/playtofile.idl
