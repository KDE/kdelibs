kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kio_file_PART_SRCS
file.cc
)

kde4_automoc(${kio_file_PART_SRCS})

kde4_add_plugin(kio_file ${kio_file_PART_SRCS})

target_link_libraries(kio_file  ${QT_AND_KDECORE_LIBS} kio )

install_targets(/lib/kde4 kio_file )


########### install files ###############

install_files( ${KDE4_SERVICES_DIR} FILES file.protocol )
install_files( /include/kio FILES file.h )

kde4_footer()



#original Makefile.am contents follow:

### Makefile.am of kdebase/kioslave/file
#
#AM_CPPFLAGS = -D_LARGEFILE64_SOURCE
#KDE_CXXFLAGS = -UQT3_SUPPORT
#INCLUDES = $(all_includes)
#
######## Files
#
#kde_module_LTLIBRARIES = kio_file.la
#
#kio_file_la_SOURCES = file.cc
#kio_file_la_LIBADD = $(LIB_KIO)
#kio_file_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#noinst_HEADERS = file.h
#
#fileinclude_HEADERS = file.h
#fileincludedir = $(includedir)/kio
#
#METASOURCES = AUTO
#
#kdelnkdir = $(kde_servicesdir)
#kdelnk_DATA = file.protocol
