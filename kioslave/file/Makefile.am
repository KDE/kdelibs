## Makefile.am of kdebase/kioslave/file

AM_CPPFLAGS = -D_LARGEFILE64_SOURCE

INCLUDES = $(all_includes)

####### Files

kde_module_LTLIBRARIES = kio_file.la

kio_file_la_SOURCES = file.cc
kio_file_la_LIBADD = $(LIB_KIO)
kio_file_la_LDFLAGS = -module $(KDE_PLUGIN)
noinst_HEADERS = file.h

fileinclude_HEADERS = file.h
fileincludedir = $(includedir)/kio

METASOURCES = AUTO

kdelnkdir = $(kde_servicesdir)
kdelnk_DATA = file.protocol
