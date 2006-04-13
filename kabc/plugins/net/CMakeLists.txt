kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kabc ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kabc_net_LIB_SRCS
   resourcenet.cpp
   resourcenetconfig.cpp
)

kde4_automoc(${kabc_net_LIB_SRCS})

kde4_add_library(kabc_net SHARED ${kabc_net_LIB_SRCS})

target_link_libraries(kabc_net ${KDE4_KDECORE_LIBS} kabc kio )

set_target_properties(kabc_net PROPERTIES VERSION 1.0.0 SOVERSION 1 )

install_targets(${LIB_INSTALL_DIR} kabc_net )


########### next target ###############

set(kabc_net_plugin_PART_SRCS resourcenetplugin.cpp )

kde4_automoc(${kabc_net_plugin_PART_SRCS})

kde4_add_plugin(kabc_net_plugin ${kabc_net_plugin_PART_SRCS})

target_link_libraries(kabc_net_plugin  ${KDE4_KDECORE_LIBS} kabc_net)
set_target_properties(kabc_net_plugin PROPERTIES OUTPUT_NAME kabc_net)

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kabc_net_plugin )
install_targets(${PLUGIN_INSTALL_DIR} kabc_net_plugin )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR}/kresources/kabc FILES net.desktop )
install_files( /include/kabc FILES resourcenet.h )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/kabc -I$(top_builddir)/kabc $(all_includes)
#
## these are the headers for your project
#noinst_HEADERS = resourcenetconfig.h
#
#lib_LTLIBRARIES = libkabc_net.la
#libkabc_net_la_SOURCES = resourcenet.cpp resourcenetconfig.cpp
#libkabc_net_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) -version-info 1:0:0 -no-undefined
#libkabc_net_la_LIBADD	= $(top_builddir)/kabc/libkabc.la $(LIB_KIO)
#libkabc_net_la_COMPILE_FIRST = $(top_builddir)/kabc/addressee.h
#
#kde_module_LTLIBRARIES = kabc_net.la
#kabc_net_la_SOURCES = resourcenetplugin.cpp
#kabc_net_la_LDFLAGS	= $(all_libraries) -module $(KDE_PLUGIN)
#kabc_net_la_LIBADD	= libkabc_net.la
#
#METASOURCES = AUTO
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kabc_net.pot
#
#kabcincludedir = $(includedir)/kabc
#kabcinclude_HEADERS = resourcenet.h
#
#servicedir = $(kde_servicesdir)/kresources/kabc
#service_DATA = net.desktop
#
#resourcenetplugin.lo: ../../addressee.h
