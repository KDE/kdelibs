kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kdeprint ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kdeprint_lpdunix_PART_SRCS
kmlpdunixfactory.cpp
kmlpdunixmanager.cpp
klpdunixprinterimpl.cpp
kmlpdunixuimanager.cpp
)

kde4_automoc(${kdeprint_lpdunix_PART_SRCS})

kde4_add_plugin(kdeprint_lpdunix ${kdeprint_lpdunix_PART_SRCS})

kde4_create_libtool_file(kdeprint_lpdunix / )

target_link_libraries(kdeprint_lpdunix  ${QT_AND_KDECORE_LIBS} kdeprint_management )

install_targets(${KDE4_LIB_INSTALL_DIR}/kde4 kdeprint_lpdunix )


########### install files ###############

install_files( ${KDE4_DATA_DIR}/kdeprint/plugins FILES lpdunix.print )

kde4_footer()



#original Makefile.am contents follow:

#AM_CPPFLAGS = -D_KDEPRINT_COMPILE
#
#INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/kdeprint $(all_includes)
#
#kde_module_LTLIBRARIES = kdeprint_lpdunix.la
#
#kdeprint_lpdunix_la_SOURCES = kmlpdunixfactory.cpp kmlpdunixmanager.cpp klpdunixprinterimpl.cpp kmlpdunixuimanager.cpp
#kdeprint_lpdunix_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
#kdeprint_lpdunix_la_LIBADD = $(top_builddir)/kdeprint/management/libkdeprint_management.la
#kdeprint_lpdunix_la_METASOURCES = AUTO
#
#noinst_HEADERS = kmlpdunixmanager.h klpdunixprinterimpl.h kmlpdunixuimanager.h
#
#entry_DATA = lpdunix.print
#entrydir = $(kde_datadir)/kdeprint/plugins
