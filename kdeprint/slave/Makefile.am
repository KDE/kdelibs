SUBDIRS = pics mimetypes templates

INCLUDES= -I$(top_srcdir)/kio -I$(top_srcdir)/kdeprint -I$(top_srcdir) $(all_includes)

kde_module_LTLIBRARIES = kio_print.la

kio_print_la_SOURCES = kio_print.cpp
kio_print_la_LIBADD = $(LIB_KIO) $(top_builddir)/kdeprint/libkdeprint.la
kio_print_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)

METASOURCES = AUTO

noinst_HEADERS = kio_print.h

# Services
servicesdir = $(kde_servicesdir)
services_DATA = print.protocol
