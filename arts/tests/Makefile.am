check_PROGRAMS = testbuffer testifacerepo

TESTS = $(check_PROGRAMS)

INCLUDES = $(all_includes) -I$(top_srcdir)/arts/mcop -I$(top_srcdir)/arts/flow \
   -I$(top_builddir)/arts/flow -I$(top_srcdir)/arts/soundserver \
   -I$(top_builddir)/arts/soundserver -I$(top_srcdir)/arts/artsc

LDFLAGS   = $(all_libraries) $(top_builddir)/arts/mcop/libmcop.la

testbuffer_SOURCES = testbuffer.cc
testifacerepo_SOURCES = testifacerepo.cc
