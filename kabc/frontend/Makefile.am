INCLUDES = -I$(top_srcdir)/kabc $(all_includes)

METASOURCES = AUTO

bin_PROGRAMS = kabcfrontend

kabcfrontend_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kabcfrontend_LDADD = $(top_builddir)/kabc/libkabc.la $(LIB_KDECORE)
kabcfrontend_SOURCES = main.cpp mainWindow.ui
