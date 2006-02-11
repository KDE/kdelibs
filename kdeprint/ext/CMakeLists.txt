kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kdeprint ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kdeprint_ext_PART_SRCS
   kmextfactory.cpp
   kmextmanager.cpp
   kextprinterimpl.cpp
   kmextuimanager.cpp
)

kde4_automoc(${kdeprint_ext_PART_SRCS})

kde4_add_plugin(kdeprint_ext ${kdeprint_ext_PART_SRCS})

target_link_libraries(kdeprint_ext  ${QT_AND_KDECORE_LIBS} kdeprint )

kde4_install_libtool_file(${KDE4_PLUGIN_INSTALL_DIR} kdeprint_ext )
install_targets(${KDE4_PLUGIN_INSTALL_DIR} kdeprint_ext )


########### install files ###############

install_files( ${KDE4_DATA_DIR}/kdeprint/plugins FILES ext.print )

kde4_footer()



#original Makefile.am contents follow:

#AM_CPPFLAGS = -D_KDEPRINT_COMPILE
#
#INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/kdeprint $(all_includes)
#
#kde_module_LTLIBRARIES = kdeprint_ext.la
#
#kdeprint_ext_la_SOURCES = kmextfactory.cpp kmextmanager.cpp kextprinterimpl.cpp kmextuimanager.cpp
#kdeprint_ext_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
#kdeprint_ext_la_LIBADD = $(top_builddir)/kdeprint/libkdeprint.la
#kdeprint_ext_la_METASOURCES = AUTO
#
#noinst_HEADERS = kmextmanager.h kextprinterimpl.h kmextuimanager.h
#
#entry_DATA = ext.print
#entrydir = $(kde_datadir)/kdeprint/plugins
