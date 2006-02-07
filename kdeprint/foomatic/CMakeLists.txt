kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kdeprint ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kdeprint_foomatic_PART_SRCS
kmfoomaticfactory.cpp
kmfoomaticmanager.cpp
kfoomaticprinterimpl.cpp
)

kde4_automoc(${kdeprint_foomatic_PART_SRCS})

kde4_add_plugin(kdeprint_foomatic ${kdeprint_foomatic_PART_SRCS})

kde4_create_libtool_file(kdeprint_foomatic / )

target_link_libraries(kdeprint_foomatic  ${QT_AND_KDECORE_LIBS} kdeprint_management )

install_targets(${KDE4_LIB_INSTALL_DIR}/kde4 kdeprint_foomatic )


########### next target ###############

set(make_driver_db_foomatic_SRCS
make_driver_db_foomatic.c
)

kde4_automoc(${make_driver_db_foomatic_SRCS})

kde4_add_executable(make_driver_db_foomatic ${make_driver_db_foomatic_SRCS})

target_link_libraries(make_driver_db_foomatic  ${QT_AND_KDECORE_LIBS} )

install_targets(/bin make_driver_db_foomatic )


########### install files ###############

install_files( ${KDE4_DATA_DIR}/kdeprint/plugins FILES foomatic.print )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/kdeprint $(all_includes)
#
#kde_module_LTLIBRARIES = kdeprint_foomatic.la
#
#kdeprint_foomatic_la_SOURCES = kmfoomaticfactory.cpp \
#				  kmfoomaticmanager.cpp \
#				  kfoomaticprinterimpl.cpp
#kdeprint_foomatic_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
#kdeprint_foomatic_la_LIBADD = $(top_builddir)/kdeprint/management/libkdeprint_management.la
#kdeprint_foomatic_la_METASOURCES = AUTO
#
#noinst_HEADERS = kmfoomaticmanager.h \
#		 kfoomaticprinterimpl.h
#
#bin_PROGRAMS = make_driver_db_foomatic
#
#make_driver_db_foomatic_SOURCES = make_driver_db_foomatic.c
#
#entry_DATA = foomatic.print
#entrydir = $(kde_datadir)/kdeprint/plugins
