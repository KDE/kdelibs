SUBDIRS = pics mimetypes templates

INCLUDES= $(all_includes)

kde_module_LTLIBRARIES = kio_print.la

kio_print_la_SOURCES = kio_print.cpp
kio_print_la_LIBADD = $(LIB_KIO) -lkdeprint
kio_print_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)

METASOURCES = AUTO

noinst_HEADERS = kio_print.h

# Services
servicesdir = $(kde_servicesdir)
services_DATA = print.protocol
