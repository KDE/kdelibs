kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/libltdl ${CMAKE_SOURCE_DIR}/kdecore/network ${CMAKE_SOURCE_DIR}/kded ${CMAKE_SOURCE_DIR}/kdecore/network ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions")
#remove_definitions(-DQT3_SUPPORT)

########### next target ###############

set(kded_proxyscout_PART_SRCS
proxyscout.cpp
script.cpp
downloader.cpp
discovery.cpp
)

kde4_automoc(${kded_proxyscout_PART_SRCS})

set( kded_proxyscout_DCOP_SKEL_SRCS
proxyscout.h
)

kde4_add_dcop_skels(kded_proxyscout_PART_SRCS ${kded_proxyscout_DCOP_SKEL_SRCS})

kde4_add_plugin(kded_proxyscout ${kded_proxyscout_PART_SRCS})

target_link_libraries(kded_proxyscout  ${QT_AND_KDECORE_LIBS} kio kjs )

# this needs -l resolv e.g on Slackware, but not on FreeBSD
check_library_exists(resolv res_query "" HAVE_RESOLV_LIBRARY)
if (HAVE_RESOLV_LIBRARY)
   target_link_libraries(kded_proxyscout resolv)
endif (HAVE_RESOLV_LIBRARY)

install_targets(${KDE4_LIB_INSTALL_DIR}/kde4 kded_proxyscout )


########### next target ###############

set(kpac_dhcp_helper_SRCS
kpac_dhcp_helper.c
)

kde4_automoc(${kpac_dhcp_helper_SRCS})

kde4_add_executable(kpac_dhcp_helper ${kpac_dhcp_helper_SRCS})

target_link_libraries(kpac_dhcp_helper  ${QT_AND_KDECORE_LIBS} )

install_targets(/bin kpac_dhcp_helper )


########### install files ###############

install_files( ${KDE4_DATA_DIR}/proxyscout FILES eventsrc )
install_files( ${KDE4_SERVICES_DIR}/kded FILES proxyscout.desktop )

kde4_footer()



#original Makefile.am contents follow:

#
#kde_module_LTLIBRARIES = kded_proxyscout.la
#bin_PROGRAMS = kpac_dhcp_helper
#
#INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/libltdl -I$(top_srcdir)/kdecore/network -I$(top_srcdir)/kded -I$(top_builddir)/kjs \
#           -I$(top_srcdir)/kdecore/network $(all_includes)
#METASOURCES = AUTO
#KDE_CXXFLAGS = $(USE_EXCEPTIONS)
#
#kded_proxyscout_la_SOURCES = proxyscout.skel proxyscout.cpp script.cpp \
#                             downloader.cpp discovery.cpp
#kded_proxyscout_la_LDFLAGS = $(all_libraries) -module -avoid-version
#kded_proxyscout_la_LIBADD = $(LIB_KIO) $(top_builddir)/kjs/libkjs.la $(LIB_KDED)
#
#kpac_dhcp_helper_SOURCES = kpac_dhcp_helper.c
#kpac_dhcp_helper_CFLAGS = $(KDE_USE_FPIE)
#kpac_dhcp_helper_LDFLAGS = $(KDE_USE_PIE)
#kpac_dhcp_helper_LDADD = $(LIBSOCKET)
#
#noinst_HEADERS = proxyscout.h script.h downloader.h discovery.h
#
#servicesdir = $(kde_servicesdir)/kded
#services_DATA = proxyscout.desktop
#
#proxyscoutdatadir = $(kde_datadir)/proxyscout
#proxyscoutdata_DATA = eventsrc
#
#install-exec-local:
#	@(chown root $(DESTDIR)$(bindir)/kpac_dhcp_helper && chmod 4755 $(DESTDIR)$(bindir)/kpac_dhcp_helper) || echo "Please make kpac_dhcp_helper setuid root"
