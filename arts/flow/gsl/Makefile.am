####### Compile without glib

AM_CFLAGS = -DGSL_WANT_GLIB_WRAPPER -DGSL_WANT_ARTS_THREADS
AM_CXXFLAGS = -DGSL_WANT_GLIB_WRAPPER -DGSL_WANT_ARTS_THREADS

####### Files

# from gslmakefile.inc
GSL_C_SOURCES = gsldatacache.c gsldatahandle.c gslwavechunk.c gslwavedsc.c \
  gslmath.c gslengine.c gsloputil.c gslopmaster.c \
  gslopschedule.c gslfilter.c gslcommon.c gsldatautils.c \
  gslconvert.c gslfft.c

noinst_LTLIBRARIES = libgsl.la
INCLUDES = -I$(top_srcdir)/arts/flow -I$(top_srcdir)/arts/mcop $(all_includes) 

libgsl_la_SOURCES = $(GSL_C_SOURCES) gslglib.c gslglibhash.cc gslartsthreads.cc
  
libgsl_la_LIBADD = $(top_builddir)/arts/mcop/libmcop.la -lm
libgsl_la_LDFLAGS = -no-undefined

check_PROGRAMS = gslglibhashtest

gslglibhashtest_SOURCES = gslglibhashtest.cc
gslglibhashtest_LDFLAGS = -lgsl
