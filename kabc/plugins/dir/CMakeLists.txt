kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kabc ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kabc_dir_LIB_SRCS
resourcedir.cpp
resourcedirconfig.cpp
)

kde4_automoc(${kabc_dir_LIB_SRCS})

kde4_add_library(kabc_dir SHARED ${kabc_dir_LIB_SRCS})

target_link_libraries(kabc_dir  ${QT_AND_KDECORE_LIBS} kabc)

set_target_properties(kabc_dir PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(/lib kabc_dir )


########### next target ###############

set(kabc_directory_PART_SRCS
resourcedirplugin.cpp
)

kde4_automoc(${kabc_directory_PART_SRCS})

kde4_add_plugin(kabc_directory ${kabc_directory_PART_SRCS})

target_link_libraries(kabc_directory  ${QT_AND_KDECORE_LIBS} kabc_dir)

install_targets(/lib/kde4 kabc_directory )


########### install files ###############

install_files( ${KDE4_SERVICES_DIR}/kresources/kabc FILES dir.desktop )
install_files( /include/kabc FILES resourcedir.h )

kde4_footer()



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
