INCLUDES= $(SSL_INCLUDES) -I$(top_srcdir)/kio/kssl -I$(top_srcdir) $(all_includes)

lib_LTLIBRARIES = libkcertpart.la
KCERTPARTVERSION=	2.2.0
KCERTPARTPATCH=	0

libkcertpart_la_SOURCES	= kcertpart.cc                   

libkcertpart_la_LDFLAGS = $(all_libraries) -version-info 4:0 -no-undefined $(KDE_PLUGIN)
libkcertpart_la_LIBADD = $(LIB_KPARTS)
libkcertpart_la_DEPENDENCIES = $(LIB_KPARTS)

libkcertpart_la_METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kcertpart.desktop

rcdir = $(kde_datadir)/kcertpart
rc_DATA = kcertpart.rc

