INCLUDES= -I$(top_srcdir)/kio/kssl -I$(top_builddir)/kio/kssl -I$(top_srcdir) $(SSL_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = libkcertpart.la

libkcertpart_la_SOURCES	= kcertpart.cc

libkcertpart_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
libkcertpart_la_LIBADD = $(LIB_KPARTS)
libkcertpart_la_DEPENDENCIES = $(LIB_KPARTS)

libkcertpart_la_METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kcertpart.desktop

rcdir = $(kde_datadir)/kcertpart
rc_DATA = kcertpart.rc

