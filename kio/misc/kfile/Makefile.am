METASOURCES = AUTO

noinst_HEADERS = fileprops.h
INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/kio/kio $(all_includes)

bin_PROGRAMS = kfile
kfile_SOURCES = fileprops.cpp
kfile_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kfile_LDADD = $(top_builddir)/kio/libkio.la

