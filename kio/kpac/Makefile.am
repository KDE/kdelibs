
INCLUDES = -I$(top_srcdir) $(all_includes)

kde_module_LTLIBRARIES = libkpac.la

libkpac_la_SOURCES = kproxybindings.cpp kpac_impl.cpp

libkpac_la_LIBADD = $(LIB_KIO) $(top_builddir)/kjs/libkjs.la
libkpac_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)

noinst_HEADERS = kproxybindings.h kpac_impl.h
