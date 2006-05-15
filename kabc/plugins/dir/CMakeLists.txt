
include_directories( ${CMAKE_SOURCE_DIR}/kabc )


########### next target ###############

set(kabc_dir_SRCS resourcedirplugin.cpp resourcedir.cpp resourcedirconfig.cpp )

kde4_automoc(${kabc_dir_SRCS})

kde4_add_plugin(kabc_dir ${kabc_dir_SRCS})

target_link_libraries(kabc_dir ${KDE4_KDECORE_LIBS} kabc )

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kabc_dir )

set_target_properties(kabc_dir PROPERTIES OUTPUT_NAME kabc_directory )

install_targets(${PLUGIN_INSTALL_DIR} kabc_dir )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR}/kresources/kabc FILES dir.desktop )
install_files( /include/kabc FILES resourcedir.h )




#original Makefile.am contents follow:

#INCLUDES = -I$(top_builddir)/kabc -I$(top_srcdir)/kabc -I$(top_builddir) $(all_includes)
#
## these are the headers for your project
#noinst_HEADERS = resourcedirconfig.h
#
#lib_LTLIBRARIES = libkabc_dir.la
#libkabc_dir_la_SOURCES = resourcedir.cpp resourcedirconfig.cpp
#libkabc_dir_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) -version-info 1:0:0 -no-undefined
#libkabc_dir_la_LIBADD = $(LIB_KABC)
#libkabc_dir_la_COMPILE_FIRST = $(top_builddir)/kabc/addressee.h
#
#kde_module_LTLIBRARIES = kabc_dir.la
#kabc_dir_la_SOURCES = resourcedirplugin.cpp
#kabc_dir_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) -no-undefined
#kabc_dir_la_LIBADD = libkabc_dir.la
#
#METASOURCES = AUTO
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kabc_dir.pot
#
#kabcincludedir = $(includedir)/kabc
#kabcinclude_HEADERS = resourcedir.h
#
#servicedir = $(kde_servicesdir)/kresources/kabc
#service_DATA = dir.desktop
#
#resourcedirplugin.lo: ../../addressee.h
