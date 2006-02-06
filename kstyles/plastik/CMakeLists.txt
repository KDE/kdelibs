kde4_header()

add_subdirectory( config ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(plastik_PART_SRCS
plastik.cpp
misc.cpp
)

kde4_automoc(${plastik_PART_SRCS})

kde4_add_plugin(plastik ${plastik_PART_SRCS})

target_link_libraries(plastik  ${QT_AND_KDECORE_LIBS} kdefx )

install_targets(${KDE4_LIB_INSTALL_DIR}/kde4/plugins/styles/ plastik )


########### install files ###############

install_files( ${KDE4_DATA_DIR}/kstyle/themes FILES plastik.themerc )

kde4_footer()



#original Makefile.am contents follow:

#AUTOMAKE_OPTIONS = foreign
#
#SUBDIRS = config
#
#KDE_CXXFLAGS = -DQT_PLUGIN
#
#INCLUDES = $(all_includes)
#
#stylercdir = $(kde_datadir)/kstyle/themes
#stylerc_DATA = plastik.themerc
#
#EXTRA_DIST = $(stylerc_DATA)
#
#noinst_HEADERS = plastik.h misc.h
#
#kde_style_LTLIBRARIES = plastik.la
#plastik_la_SOURCES = plastik.cpp misc.cpp
#plastik_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
#plastik_la_LIBADD = ../../kdefx/libkdefx.la
#plastik_la_METASOURCES = AUTO
#
#DISTCLEANFILES = $(plastik_la_METASOURCES)
