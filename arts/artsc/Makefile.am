AM_CFLAGS = -DARTSC_BACKEND='"$(kde_libraries)/libartscbackend.la"'
lib_LTLIBRARIES = libartsc.la libartscbackend.la
FLOWLIBS = $(top_builddir)/arts/flow/libartsflow.la

INCLUDES = $(all_includes) -I$(top_srcdir)/arts/mcop -I$(top_srcdir)/arts/flow \
      -I$(top_builddir)/arts/flow -I$(top_builddir)/arts/soundserver \
      -I$(top_srcdir)/libltdl

bin_SCRIPTS = artsc-config

libartsc_la_SOURCES = artsc.c
libartsc_la_LDFLAGS = -no-undefined
libartsc_la_LIBADD = $(top_builddir)/libltdl/libltdlc.la

libartscbackend_la_SOURCES = artscbackend.cc
libartscbackend_la_LDFLAGS = -no-undefined -module
libartscbackend_la_LIBADD = $(FLOWLIBS) \
    $(top_builddir)/arts/soundserver/libsoundserver_idl.la

artsincludedir = $(includedir)/arts
artsinclude_HEADERS = artsc.h
