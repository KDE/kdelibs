# $Id$

INCLUDES = -I$(top_builddir)/kdedb -I$(PG_INCDIR) $(all_includes)

METASOURCES = AUTO

lib_LTLIBRARIES = libkdb_postgres.la
libkdb_postgres_la_SOURCES = 	connectorimpl.cpp factory.cpp pluginimpl.cpp \
				handlerimpl.cpp controlimpl.cpp
libkdb_postgres_la_LDFLAGS = -L$(top_builddir)/kdedb/kdbcore -L$(PG_LIBDIR) $(all_libraries) $(KDE_PLUGIN)
libkdb_postgres_la_LIBADD = $(LIB_KDBCORE) -lpq

noinst_HEADERS = connectorimpl.h controlimpl.h factory.h handlerimpl.h pluginimpl.h

datadir = $(kde_servicesdir)
data_DATA = kdb_postgres.desktop
