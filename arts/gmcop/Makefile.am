lib_LTLIBRARIES = libgmcop.la

GLIB_PREFIX = /usr/local

INCLUDES = -I$(top_srcdir)/arts/mcop $(GLIB_CFLAGS) $(all_includes)

# $(GLIB_DEPLIBS) ?
libgmcop_la_SOURCES = giomanager.cc
libgmcop_la_LIBADD = $(top_builddir)/arts/mcop/libmcop.la $(GLIB_LIBS)
libgmcop_la_LDFLAGS = -no-undefined -L$(GLIB_PREFIX)/lib -version-info 1:0

artsincludedir = $(includedir)/arts
artsinclude_HEADERS = giomanager.h
