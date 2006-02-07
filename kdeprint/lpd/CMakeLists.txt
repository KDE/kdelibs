kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kdeprint ${CMAKE_SOURCE_DIR}/kdeprint/management ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kdeprint_lpd_PART_SRCS
kmlpdfactory.cpp
kmlpdmanager.cpp
klpdprinterimpl.cpp
kmlpduimanager.cpp
lpdtools.cpp
gschecker.cpp
)

kde4_automoc(${kdeprint_lpd_PART_SRCS})

kde4_add_plugin(kdeprint_lpd ${kdeprint_lpd_PART_SRCS})

kde4_create_libtool_file(kdeprint_lpd /kde4/ )

target_link_libraries(kdeprint_lpd  ${QT_AND_KDECORE_LIBS} kdeprint_management )

install_targets(${KDE4_LIB_INSTALL_DIR}/kde4 kdeprint_lpd )


########### next target ###############

set(make_driver_db_lpd_SRCS
make_driver_db_lpd.c
)

kde4_automoc(${make_driver_db_lpd_SRCS})

kde4_add_executable(make_driver_db_lpd ${make_driver_db_lpd_SRCS})

target_link_libraries(make_driver_db_lpd  ${QT_AND_KDECORE_LIBS} kdefakes )

install_targets(/bin make_driver_db_lpd )


########### install files ###############

install_files( ${KDE4_DATA_DIR}/kdeprint/plugins FILES lpd.print )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/kdeprint -I$(top_srcdir)/kdeprint/management $(all_includes)
#
#kde_module_LTLIBRARIES = kdeprint_lpd.la
#
#kdeprint_lpd_la_SOURCES = kmlpdfactory.cpp kmlpdmanager.cpp klpdprinterimpl.cpp kmlpduimanager.cpp \
#			     lpdtools.cpp gschecker.cpp
#kdeprint_lpd_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
#kdeprint_lpd_la_LIBADD = $(top_builddir)/kdeprint/management/libkdeprint_management.la 
#kdeprint_lpd_la_METASOURCES = AUTO
#
#noinst_HEADERS = kmlpdfactory.h kmlpdmanager.h klpdprinterimpl.h kmlpduimanager.h lpdtools.h gschecker.h
#
#bin_PROGRAMS = make_driver_db_lpd
#make_driver_db_lpd_SOURCES = make_driver_db_lpd.c
#make_driver_db_lpd_LDADD = $(top_builddir)/kdecore/libkdefakes.la
#
#entry_DATA = lpd.print
#entrydir = $(kde_datadir)/kdeprint/plugins
