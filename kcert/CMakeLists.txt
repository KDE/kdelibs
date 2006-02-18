kde4_header()

project(kcert)

include_directories( ${CMAKE_SOURCE_DIR}/kio/kssl ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )

include_directories(${OPENSSL_INCLUDE_DIR} ${CMAKE_BINARY_DIR}/kio/kssl )

########### next target ###############

set(kcertpart_PART_SRCS kcertpart.cc )

kde4_automoc(${kcertpart_PART_SRCS})

kde4_add_plugin(kcertpart WITH_PREFIX ${kcertpart_PART_SRCS})

target_link_libraries(kcertpart  ${QT_AND_KDECORE_LIBS} kparts )

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kcertpart )
install_targets(${PLUGIN_INSTALL_DIR} kcertpart )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kcertpart.desktop )
install_files( ${DATA_INSTALL_DIR}/kcertpart FILES kcertpart.rc )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES= -I$(top_srcdir)/kio/kssl -I$(top_builddir)/kio/kssl -I$(top_srcdir) $(SSL_INCLUDES) $(all_includes)
#
#kde_module_LTLIBRARIES = libkcertpart.la
#
#libkcertpart_la_SOURCES	= kcertpart.cc
#
#libkcertpart_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
#libkcertpart_la_LIBADD = $(LIB_KPARTS)
#libkcertpart_la_DEPENDENCIES = $(LIB_KPARTS)
#
#libkcertpart_la_METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kcertpart.desktop
#
#rcdir = $(kde_datadir)/kcertpart
#rc_DATA = kcertpart.rc
#
