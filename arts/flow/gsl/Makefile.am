####### Compile without glib

AM_CFLAGS = -DGSL_WANT_GLIB_WRAPPER -DGSL_WANT_ARTS_THREADS
AM_CXXFLAGS = -DGSL_WANT_GLIB_WRAPPER -DGSL_WANT_ARTS_THREADS

####### Files

noinst_LTLIBRARIES = libgsl.la
INCLUDES = -I$(top_srcdir)/arts/flow -I$(top_srcdir)/arts/mcop $(all_includes) 

libgsl_la_SOURCES = gslcommon.c gsldatacache.c gslengine.c gslfilter.c \
  gslmath.c gslopmaster.c gslopschedule.c gsloputil.c gslwavechunk.c \
  gslwavedsc.c gslwaveloader.c gslglib.c gslglibhash.cc gslartsthreads.cc \
  gsldatahandle.c
libgsl_la_LIBADD = $(top_builddir)/arts/mcop/libmcop.la -lm
libgsl_la_LDFLAGS = -no-undefined

check_PROGRAMS = gslglibhashtest

gslglibhashtest_SOURCES = gslglibhashtest.cc
gslglibhashtest_LDFLAGS = -lgsl
