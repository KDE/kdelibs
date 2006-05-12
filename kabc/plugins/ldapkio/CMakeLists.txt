
include_directories( ${CMAKE_SOURCE_DIR}/kabc )


########### next target ###############

set(kabc_ldapkio_LIB_SRCS resourceldapkio.cpp resourceldapkioconfig.cpp )

kde4_automoc(${kabc_ldapkio_LIB_SRCS})

kde4_add_library(kabc_ldapkio SHARED ${kabc_ldapkio_LIB_SRCS})

target_link_libraries(kabc_ldapkio  ${KDE4_KDECORE_LIBS} kabc kio )

set_target_properties(kabc_ldapkio PROPERTIES VERSION 4.0.0 SOVERSION 4 )

install_targets(${LIB_INSTALL_DIR} kabc_ldapkio )


########### next target ###############

set(kabc_ldapkio_plugin_PART_SRCS resourceldapkioplugin.cpp )

kde4_automoc(${kabc_ldapkio_plugin_PART_SRCS})

kde4_add_plugin(kabc_ldapkio_plugin ${kabc_ldapkio_plugin_PART_SRCS})

target_link_libraries(kabc_ldapkio_plugin  ${KDE4_KDECORE_LIBS} kabc_ldapkio )
set_target_properties(kabc_ldapkio_plugin PROPERTIES OUTPUT_NAME kabc_ldapkio)
kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kabc_ldapkio_plugin )
install_targets(${PLUGIN_INSTALL_DIR} kabc_ldapkio_plugin )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR}/kresources/kabc FILES ldapkio.desktop )
install_files( /include/kabc FILES resourceldapkio.h )




#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/kabc -I$(top_builddir)/kabc $(all_includes)
#
## these are the headers for your project
#noinst_HEADERS = resourceldapkioconfig.h
#
#lib_LTLIBRARIES = libkabc_ldapkio.la
#libkabc_ldapkio_la_SOURCES = resourceldapkio.cpp resourceldapkioconfig.cpp
#libkabc_ldapkio_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) -version-info 1:0:0 -no-undefined
#libkabc_ldapkio_la_LIBADD	= $(top_builddir)/kabc/libkabc.la $(LIB_KIO)
#libkabc_ldapkio_la_COMPILE_FIRST = $(top_builddir)/kabc/addressee.h
#
#kde_module_LTLIBRARIES = kabc_ldapkio.la
#kabc_ldapkio_la_SOURCES = resourceldapkioplugin.cpp
#kabc_ldapkio_la_LDFLAGS	= $(all_libraries) -module $(KDE_PLUGIN)
#kabc_ldapkio_la_LIBADD	= libkabc_ldapkio.la
#
#METASOURCES = AUTO
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kabc_ldapkio.pot
#
#kabcincludedir = $(includedir)/kabc
#kabcinclude_HEADERS = resourceldapkio.h
#
#servicedir = $(kde_servicesdir)/kresources/kabc
#service_DATA = ldapkio.desktop
#
#resourceldapkioplugin.lo: ../../addressee.h
