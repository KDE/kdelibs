
EXTRA_DIST = $(GSL_PRIVATE_H_SRC) $(GSL_H_SRC)

include gslmakefile.inc

####### Compile without glib

AM_CFLAGS = -DGSL_WANT_GLIB_WRAPPER -DGSL_WANT_ARTS_THREADS
AM_CXXFLAGS = -DGSL_WANT_GLIB_WRAPPER -DGSL_WANT_ARTS_THREADS

INCLUDES = -I$(top_srcdir)/arts/flow -I$(top_srcdir)/arts/mcop $(all_includes) 

noinst_LTLIBRARIES = libgsl.la

libgsl_la_SOURCES = $(GSL_C_SRC) gslglib.c gslglibhash.cc gslartsthreads.cc
libgsl_la_LIBADD = $(top_builddir)/arts/mcop/libmcop.la -lm $(LIBPOSIX4)
libgsl_la_LDFLAGS = -no-undefined

noinst_PROGRAMS = $(GSL_NOINST_PROGS)

progs_ldadd = libgsl.la
