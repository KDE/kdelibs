## $Id$
## Makefile.am of kdebase/kioslave/metainfo

INCLUDES = $(all_includes)
LDFLAGS = $(all_libraries) $(KDE_RPATH)
METASOURCES = AUTO

kde_module_LTLIBRARIES = kio_metainfo.la

kio_metainfo_la_SOURCES = metainfo.cpp
kio_metainfo_la_LIBADD = $(LIB_KIO)
kio_metainfo_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)

noinst_HEADERS = metainfo.h 

kdelnk_DATA = metainfo.protocol
kdelnkdir = $(kde_servicesdir)

#servicetypes_DATA = thumbcreator.desktop
#servicetypesdir = $(kde_servicetypesdir)

#services_DATA = imagethumbnail.desktop textthumbnail.desktop 
#                htmlthumbnail.desktop gsthumbnail.desktop
#servicesdir = $(kde_servicesdir)
