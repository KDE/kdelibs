INCLUDES= $(all_includes)

####### Files

kde_module_LTLIBRARIES = kio_mycomputer.la

kio_mycomputer_la_SOURCES = mycomputer.cc
kio_mycomputer_la_LIBADD  = $(LIB_KSYCOCA)
kio_mycomputer_la_LDFLAGS = -module $(KDE_PLUGIN)

noinst_HEADERS = mycomputer.h

kdelnk_DATA = mycomputer.protocol
kdelnkdir = $(kde_servicesdir)


SUBDIRS=. kdedmodule mimetypes
