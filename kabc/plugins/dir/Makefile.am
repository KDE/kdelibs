INCLUDES = -I$(top_builddir)/kabc -I$(top_srcdir)/kabc -I$(top_builddir) $(all_includes)

# these are the headers for your project
noinst_HEADERS   = resourcedir.h resourcedirconfig.h

kde_module_LTLIBRARIES = kabc_dir.la

kabc_dir_la_SOURCES = resourcedir.cpp resourcedirconfig.cpp

kabc_dir_la_LDFLAGS	= $(all_libraries) -module $(KDE_PLUGIN)
kabc_dir_la_LIBADD	= $(LIB_KABC)
kabc_dir_la_COMPILE_FIRST = $(top_builddir)/kabc/addressee.h

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kabc_dir.pot

servicedir = $(kde_servicesdir)/kresources/kabc
service_DATA = dir.desktop
