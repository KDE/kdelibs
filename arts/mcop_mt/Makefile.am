INCLUDES = -I$(top_srcdir)/arts/mcop $(all_includes)

lib_LTLIBRARIES = libmcop_mt.la

libmcop_mt_la_SOURCES = threads_posix.cc
libmcop_mt_la_LIBADD = $(top_builddir)/arts/mcop/libmcop.la $(LIBPTHREAD)
libmcop_mt_la_LDFLAGS = -no-undefined $(USE_THREADS) 
