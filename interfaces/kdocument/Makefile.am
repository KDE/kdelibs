INCLUDES = -I$(top_srcdir)/interfaces $(all_includes)

lib_LTLIBRARIES = libkdocument.la

libkdocument_la_SOURCES = kdocument.cpp

libkdocument_la_LDFLAGS = -no-undefined $(all_libraries) $(KDE_RPATH)
libkdocument_la_LIBADD = $(LIB_KPARTS)

kdocumentinclude_HEADERS = factory.h document.h view.h

kdocumentincludedir = $(includedir)/kdocument

METASOURCES = AUTO

#servicetypedir = $(kde_servicetypesdir)
#servicetype_DATA = kdocument.desktop kdocumentplugin.desktop

#kcm_kdocument_DATA = kcm_kdocument.desktop
#kcm_kdocumentdir = $(kde_datadir)/kcm_componentchooser

DOXYGEN_REFERENCES = kparts
include ../../admin/Doxyfile.am

