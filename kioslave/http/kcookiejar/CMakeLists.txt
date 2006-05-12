
add_subdirectory( tests ) 

# all done by parent dir
# include_directories( ${KDE4_KIO_INCLUDES} )


########### next target ###############

set(kcookiejar_SRCS main.cpp)

kde4_automoc(${kcookiejar_SRCS})

kde4_add_executable( kcookiejar NOGUI ${kcookiejar_SRCS})

target_link_libraries( kcookiejar  ${KDE4_KDECORE_LIBS} )

install(TARGETS kcookiejar DESTINATION bin)

########### next target ###############

set(kded_kcookiejar_PART_SRCS
   kcookiejar.cpp
   kcookieserver.cpp
   kcookiewin.cpp
)

kde4_automoc(${kded_kcookiejar_PART_SRCS})

set( kded_kcookiejar_DCOP_SKEL_SRCS kcookieserver.h )

kde4_add_dcop_skels(kded_kcookiejar_PART_SRCS ${kded_kcookiejar_DCOP_SKEL_SRCS})

kde4_add_plugin(kded_kcookiejar ${kded_kcookiejar_PART_SRCS})

target_link_libraries(kded_kcookiejar  ${KDE4_KDECORE_LIBS} kio )

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kded_kcookiejar )
install_targets(${PLUGIN_INSTALL_DIR} kded_kcookiejar )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/khtml FILES domain_info )
install_files( ${SERVICES_INSTALL_DIR}/kded FILES kcookiejar.desktop )
install_files( ${DATA_INSTALL_DIR}/kconf_update FILES kcookiescfg.upd )




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
