kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kio_metainfo_PART_SRCS
metainfo.cpp
)

kde4_automoc(${kio_metainfo_PART_SRCS})

kde4_add_plugin(kio_metainfo ${kio_metainfo_PART_SRCS})


target_link_libraries(kio_metainfo  ${QT_AND_KDECORE_LIBS} kio )

kde4_install_libtool_file(${KDE4_PLUGIN_INSTALL_DIR} kio_metainfo )
install_targets(${KDE4_PLUGIN_INSTALL_DIR} kio_metainfo )


########### install files ###############

install_files( ${KDE4_SERVICES_DIR} FILES metainfo.protocol )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#AM_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#METASOURCES = AUTO
#KDE_CXXFLAGS = -UQT3_SUPPORT
#
#kde_module_LTLIBRARIES = kio_metainfo.la
#
#kio_metainfo_la_SOURCES = metainfo.cpp
#kio_metainfo_la_LIBADD = $(LIB_KIO)
#kio_metainfo_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#
#noinst_HEADERS = metainfo.h 
#
#kdelnk_DATA = metainfo.protocol
#kdelnkdir = $(kde_servicesdir)
#
##servicetypes_DATA = thumbcreator.desktop
##servicetypesdir = $(kde_servicetypesdir)
#
##services_DATA = imagethumbnail.desktop textthumbnail.desktop 
##                htmlthumbnail.desktop gsthumbnail.desktop
##servicesdir = $(kde_servicesdir)
