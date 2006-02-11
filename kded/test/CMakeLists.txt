kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kded_test_PART_SRCS test.cpp )

kde4_automoc(${kded_test_PART_SRCS})

set( kded_test_DCOP_SKEL_SRCS test.h )

kde4_add_dcop_skels(kded_test_PART_SRCS ${kded_test_DCOP_SKEL_SRCS})

kde4_add_plugin(kded_test ${kded_test_PART_SRCS})

target_link_libraries(kded_test  ${QT_AND_KDECORE_LIBS} )

kde4_install_libtool_file(${KDE4_PLUGIN_INSTALL_DIR} kded_test )
install_targets(${KDE4_PLUGIN_INSTALL_DIR} kded_test )


########### install files ###############

install_files( ${KDE4_SERVICES_DIR}/kded FILES test.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES= -I$(srcdir)/.. $(all_includes) 
#
######## Files
#
#kde_module_LTLIBRARIES = kded_test.la
#
#kded_test_la_SOURCES = test.cpp test.skel
#kded_test_la_METASOURCES = AUTO
#kded_test_la_LDFLAGS = $(all_libraries) -module -avoid-version
#kded_test_la_LIBADD = $(LIB_KDECORE)
#
#servicesdir = $(kde_servicesdir)/kded
#services_DATA = test.desktop
#
