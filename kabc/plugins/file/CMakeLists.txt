
include_directories( ${CMAKE_SOURCE_DIR}/kabc )


########### kabc_file_core library (also used by kaddressbook) ###############

set(kabc_file_core_SRCS resourcefile.cpp resourcefileconfig.cpp )

kde4_automoc(${kabc_file_core_SRCS})

kde4_add_library(kabc_file_core SHARED ${kabc_file_core_SRCS})

target_link_libraries(kabc_file_core ${KDE4_KDECORE_LIBS} kabc)

set_target_properties(kabc_file_core PROPERTIES VERSION 4.0.0 SOVERSION 4 )

install_targets(${LIB_INSTALL_DIR} kabc_file_core )


########### kabc_file plugin ###############

set(kabc_file_SRCS resourcefileplugin.cpp)

kde4_automoc(${kabc_file_SRCS})

kde4_add_plugin(kabc_file ${kabc_file_SRCS})

target_link_libraries(kabc_file ${KDE4_KDECORE_LIBS} kabc_file_core)

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kabc_file )

install_targets(${PLUGIN_INSTALL_DIR} kabc_file )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR}/kresources/kabc FILES file.desktop )
install_files( /include/kabc FILES resourcefile.h )




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
