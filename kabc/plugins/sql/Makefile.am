INCLUDES = -I$(top_srcdir)/kabc -I$(top_builddir)/kabc $(all_includes)

# these are the headers for your project
noinst_HEADERS   = resourcesql.h resourcesqlconfig.h

kde_module_LTLIBRARIES = kabc_sql.la

kabc_sql_la_SOURCES = resourcesql.cpp resourcesqlconfig.cpp

kabc_sql_la_LDFLAGS	= $(all_libraries) -module $(KDE_PLUGIN)
kabc_sql_la_LIBADD	= ../../libkabc.la ../../../kdeui/libkdeui.la

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kabc_sql.pot

linkdir		= $(kde_datadir)/kresources/contact
link_DATA	= sql.desktop
EXTRA_DIST	= $(link_DATA)
