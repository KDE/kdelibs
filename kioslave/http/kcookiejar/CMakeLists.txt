kde4_header()

add_subdirectory( tests ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kcookiejar_KDEINIT_SRCS
main.cpp
)

kde4_automoc(${kcookiejar_KDEINIT_SRCS})

kde4_add_kdeinit_executable( kcookiejar ${kcookiejar_KDEINIT_SRCS})

target_link_libraries(kdeinit_kcookiejar  ${QT_AND_KDECORE_LIBS} )

install_targets(${KDE4_LIB_INSTALL_DIR} kdeinit_kcookiejar )

target_link_libraries( kcookiejar kdeinit_kcookiejar )
install_targets(/bin kcookiejar )

########### next target ###############

set(kded_kcookiejar_PART_SRCS
kcookiejar.cpp
kcookieserver.cpp
kcookiewin.cpp
)

kde4_automoc(${kded_kcookiejar_PART_SRCS})

set( kded_kcookiejar_DCOP_SKEL_SRCS
kcookieserver.h
)

kde4_add_dcop_skels(kded_kcookiejar_PART_SRCS ${kded_kcookiejar_DCOP_SKEL_SRCS})

kde4_add_plugin(kded_kcookiejar ${kded_kcookiejar_PART_SRCS})

kde4_create_libtool_file(kded_kcookiejar /kde4/ )

target_link_libraries(kded_kcookiejar  ${QT_AND_KDECORE_LIBS} kio )

install_targets(${KDE4_LIB_INSTALL_DIR}/kde4 kded_kcookiejar )


########### install files ###############

install_files( ${KDE4_DATA_DIR}/khtml FILES domain_info )
install_files( ${KDE4_SERVICES_DIR}/kded FILES kcookiejar.desktop )
install_files( ${KDE4_DATA_DIR}/kconf_update FILES kcookiescfg.upd )

kde4_footer()



#original Makefile.am contents follow:

## Makefile.am of kdebase/kioslave/http
#
#SUBDIRS=tests
#INCLUDES=	$(all_includes)
#
######## Files
#
#bin_PROGRAMS =
#lib_LTLIBRARIES =
#kdeinit_LTLIBRARIES = kcookiejar.la
#kde_module_LTLIBRARIES = kded_kcookiejar.la
#
#kcookiejar_la_SOURCES = main.cpp
#METASOURCES = AUTO
#kcookiejar_la_LDFLAGS = $(all_libraries) -module -avoid-version
#kcookiejar_la_LIBADD = $(LIB_KDECORE)
#
#kded_kcookiejar_la_SOURCES = kcookiejar.cpp kcookieserver.cpp \
#			kcookieserver.skel kcookiewin.cpp 
#kded_kcookiejar_la_LDFLAGS = $(all_libraries) -module -avoid-version
#kded_kcookiejar_la_LIBADD = $(LIB_KIO) $(LIB_KDED)
#
#kded_DATA = kcookiejar.desktop
#kdeddir = $(kde_servicesdir)/kded
#
#update_DATA = kcookiescfg.upd
#updatedir = $(kde_datadir)/kconf_update
#
#cookie_DATA = domain_info
#cookiedir = $(kde_datadir)/khtml
#
