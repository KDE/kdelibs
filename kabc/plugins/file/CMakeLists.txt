kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kabc ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kabc_file_LIB_SRCS
resourcefile.cpp
resourcefileconfig.cpp
resourcefileplugin.cpp
)

kde4_automoc(${kabc_file_LIB_SRCS})

kde4_add_library(kabc_file SHARED ${kabc_file_LIB_SRCS})
target_link_libraries(kabc_file  ${QT_AND_KDECORE_LIBS} kabc)

kde4_install_libtool_file(${LIB_INSTALL_DIR} kabc_file )
install_targets(${LIB_INSTALL_DIR} kabc_file )


########### next target ###############

set(kabc_file_PART_SRCS dummy.cpp )

kde4_automoc(${kabc_file_PART_SRCS})

kde4_add_plugin(kabc_file ${kabc_file_PART_SRCS})

target_link_libraries(kabc_file  ${QT_AND_KDECORE_LIBS} )

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kabc_file )
install_targets(${PLUGIN_INSTALL_DIR} kabc_file )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR}/kresources/kabc FILES file.desktop )
install_files( /include/kabc FILES resourcefile.h )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_builddir)/kabc -I$(top_srcdir)/kabc $(all_includes)
#
## these are the headers for your project
#noinst_HEADERS   = resourcefileconfig.h
#
#lib_LTLIBRARIES = libkabc_file.la
#libkabc_file_la_SOURCES = resourcefile.cpp resourcefileconfig.cpp
#libkabc_file_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) -version-info 1:0:0 -no-undefined
#libkabc_file_la_LIBADD = $(LIB_KABC)
#libkabc_file_la_COMPILE_FIRST = $(top_builddir)/kabc/addressee.h
#
#
#kde_module_LTLIBRARIES = kabc_file.la
#kabc_file_la_SOURCES = resourcefileplugin.cpp
#kabc_file_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) -no-undefined
#kabc_file_la_LIBADD = libkabc_file.la
#kabc_file_la_COMPILE_FIRST = $(top_builddir)/kabc/addressee.h
#
#METASOURCES = AUTO
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kabc_file.pot
#
#kabcincludedir = $(includedir)/kabc
#kabcinclude_HEADERS = resourcefile.h
#
#servicedir = $(kde_servicesdir)/kresources/kabc
#service_DATA = file.desktop
