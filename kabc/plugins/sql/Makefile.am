INCLUDES = -I$(top_srcdir)/kabc $(all_includes)

# these are the headers for your project
noinst_HEADERS   = resourcesql.h resourcesqlconfig.h

kde_module_LTLIBRARIES = kabc_sql.la

kabc_sql_la_SOURCES = resourcesql.cpp resourcesqlconfig.cpp

kabc_sql_la_LDFLAGS	= $(all_libraries) -module $(KDE_PLUGIN)
kabc_sql_la_LIBADD	= ../../libkabc.la ../../../kdeui/libkdeui.la

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kabc_sql.pot

linkdir		= $(kde_datadir)/kabc/plugins
link_DATA	= sql.desktop
EXTRA_DIST	= $(link_DATA)
