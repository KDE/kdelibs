AM_CFLAGS = -DARTSC_BACKEND='"$(libdir)/libartscbackend.la"'
lib_LTLIBRARIES = libartsc.la libartscbackend.la libartsdsp.la libartsdsp_st.la
FLOWLIBS = $(top_builddir)/arts/flow/libartsflow.la

INCLUDES = -I$(top_srcdir)/arts/mcop -I$(top_srcdir)/arts/flow \
      -I$(top_builddir)/arts/flow -I$(top_builddir)/arts/soundserver \
      -I$(top_srcdir)/libltdl $(all_includes)

bin_SCRIPTS = artsc-config artsdsp

libartsdsp_la_SOURCES = artsdsp.c
libartsdsp_la_LDFLAGS = -no-undefined -module
libartsdsp_la_LIBADD = libartsc.la

libartsdsp_st_la_SOURCES = artsc.c artsdsp.c
libartsdsp_st_la_LDFLAGS = -no-undefined -module
libartsdsp_st_la_LIBADD = $(top_builddir)/libltdl/libltdlc.la

libartsc_la_SOURCES = artsc.c
libartsc_la_LDFLAGS = -no-undefined
libartsc_la_LIBADD = $(top_builddir)/libltdl/libltdlc.la $(LIBPTHREAD) $(USE_THREADS)

libartscbackend_la_SOURCES = artscbackend.cc
libartscbackend_la_LDFLAGS = -no-undefined -module
libartscbackend_la_LIBADD = $(FLOWLIBS) \
    $(top_builddir)/arts/soundserver/libsoundserver_idl.la

artscincludedir = $(includedir)/artsc
artscinclude_HEADERS = artsc.h
