INCLUDES = -I$(top_builddir)/kdedb $(all_includes)

METASOURCES = AUTO


lib_LTLIBRARIES = libkdb_mysql.la

libkdb_mysql_la_SOURCES = connectorimpl.cpp factory.cpp pluginimpl.cpp handlerimpl.cpp

libkdb_mysql_la_LDFLAGS = -L$(top_builddir)/kdedb/kdbcore -L$(MYSQL_LIBDIR) $(all_libraries) $(KDE_PLUGIN)

libkdb_mysql_la_LIBADD = $(LIB_KDBCORE) -lmysqlclient


datadir = $(kde_servicesdir)
data_DATA = kdb_mysql.desktop 
