INCLUDES=$(all_includes) $(SSL_INCLUDES) -I$(top_builddir)/kfile -I$(top_builddir)/kio

lib_LTLIBRARIES = libkcertpart.la
KCERTPARTVERSION=	2.2.0
KCERTPARTPATCH=	0
METASOURCES= AUTO


libkcertpart_la_SOURCES	= \
			kcertpart.cc                   \
			kcertpartfactory.cc

libkcertpart_la_LDFLAGS = $(all_libraries) -version-info 4:0 -no-undefined $(KDE_PLUGIN)
libkcertpart_la_LIBADD = $(LIB_KDEUI) $(LIB_KPARTS)
libkcertpart_la_DEPENDENCIES = $(LIB_KDEUI) $(LIB_KPARTS)

libkcertpart_la_METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kcertpart.desktop

rcdir = $(kde_datadir)/kcertpart
rc_DATA = kcertpart.rc

SUBDIRS =

