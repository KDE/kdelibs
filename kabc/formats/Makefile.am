INCLUDES = -I$(top_srcdir)/kabc $(all_includes)

# these are the headers for your project
noinst_HEADERS   = binaryformat.h

kde_module_LTLIBRARIES = kabcformat_binary.la

kabcformat_binary_la_SOURCES = binaryformat.cpp

kabcformat_binary_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kabcformat_binary_la_LIBADD  = ../libkabc.la

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kabcformat_binary.pot

linkdir		= $(kde_datadir)/kabc/formats
link_DATA	= binary.desktop
EXTRA_DIST	= $(link_DATA)
