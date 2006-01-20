INCLUDES = -I$(top_srcdir)/kabc -I$(top_builddir)/kabc $(all_includes)
KDE_CXXFLAGS = -UQT3_SUPPORT

kde_module_LTLIBRARIES = kabcformat_binary.la

kabcformat_binary_la_SOURCES = binaryformat.cpp
kabcformat_binary_la_LDFLAGS =  -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) \
                                -no-undefined
kabcformat_binary_la_LIBADD  = $(top_builddir)/kabc/libkabc.la
kabcformat_binary_la_COMPILE_FIRST = $(top_builddir)/kabc/addressee.h

# these are the headers for your project
noinst_HEADERS = binaryformat.h

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kabcformat_binary.pot

linkdir = $(kde_datadir)/kabc/formats
link_DATA = binary.desktop
EXTRA_DIST = $(link_DATA)
