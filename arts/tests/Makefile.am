check_PROGRAMS = testbuffer testifacerepo testanyref testwrapper \
   testchangenotify testflowsystem testdispatcher testnotification \
   testremote

TESTS = $(check_PROGRAMS)

INCLUDES = -I$(top_srcdir)/arts/mcop -I$(top_srcdir)/arts/flow \
   -I$(top_builddir)/arts/flow -I$(top_srcdir)/arts/soundserver \
   -I$(top_builddir)/arts/soundserver -I$(top_srcdir)/arts/artsc \
    $(all_includes)

LDFLAGS   = $(all_libraries) $(top_builddir)/arts/mcop/libmcop.la
FLOWLIBS = $(LDFLAGS) $(top_builddir)/arts/flow/libartsflow.la

testbuffer_SOURCES = testbuffer.cc
testifacerepo_SOURCES = testifacerepo.cc
testanyref_SOURCES = testanyref.cc
testdispatcher_SOURCES = testdispatcher.cc
testnotification_SOURCES = testnotification.cc
testwrapper_SOURCES = wrapper.cc testwrapper.cc

testchangenotify_LDFLAGS = $(FLOWLIBS)
testchangenotify_SOURCES = value.cc value_impl.cc testchangenotify.cc

testflowsystem_LDFLAGS = $(FLOWLIBS)
testflowsystem_SOURCES = value.cc value_impl.cc testflowsystem.cc

testremote_LDFLAGS = $(FLOWLIBS)
testremote_SOURCES = remotetest.cc value.cc value_impl.cc testremote.cc

DISTCLEANFILES = wrapper.h wrapper.cc value.h value.cc \
				 remotetest.h remotetest.cc

wrapper.h: $(top_srcdir)/arts/tests/wrapper.idl $(MCOPIDL)
	$(MCOPIDL) $(top_srcdir)/arts/tests/wrapper.idl

wrapper.cc: wrapper.h

value.h: $(top_srcdir)/arts/tests/value.idl $(MCOPIDL)
	$(MCOPIDL) $(top_srcdir)/arts/tests/value.idl

value.cc: value.h

remotetest.h: $(top_srcdir)/arts/tests/remotetest.idl $(MCOPIDL)
	$(MCOPIDL) $(top_srcdir)/arts/tests/remotetest.idl

remotetest.cc: remotetest.h
