
include_directories( ${CMAKE_SOURCE_DIR}/kabc )


########### next target ###############

set(kabc_net_SRCS resourcenetplugin.cpp resourcenet.cpp resourcenetconfig.cpp )

kde4_automoc(${kabc_net_SRCS})

kde4_add_plugin(kabc_net ${kabc_net_SRCS})

target_link_libraries(kabc_net  ${KDE4_KDECORE_LIBS} kabc kio)

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kabc_net )

install_targets(${PLUGIN_INSTALL_DIR} kabc_net )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR}/kresources/kabc FILES net.desktop )
install_files( /include/kabc FILES resourcenet.h )




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
