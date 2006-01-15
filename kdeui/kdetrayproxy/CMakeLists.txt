kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kded ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kded_kdetrayproxy_PART_SRCS
kdetrayproxy.cpp
module.cpp
)

kde4_automoc(${kded_kdetrayproxy_PART_SRCS})

set( kded_kdetrayproxy_DCOP_SKEL_SRCS
module.h
)

kde4_add_dcop_skels(kded_kdetrayproxy_PART_SRCS ${kded_kdetrayproxy_DCOP_SKEL_SRCS})

kde4_add_plugin(kded_kdetrayproxy ${kded_kdetrayproxy_PART_SRCS})

target_link_libraries(kded_kdetrayproxy  ${QT_AND_KDECORE_LIBS} )

install_targets(/lib/kde4 kded_kdetrayproxy )


########### install files ###############

install_files( ${KDE4_SERVICES_DIR}/kded FILES kdetrayproxy.desktop )

kde4_footer()



#original Makefile.am contents follow:

#kde_module_LTLIBRARIES = kded_kdetrayproxy.la
#KDE_CXXFLAGS = -UQT3_SUPPORT
#
#INCLUDES= -I$(top_srcdir)/kded $(all_includes)
#kded_kdetrayproxy_la_SOURCES = kdetrayproxy.cpp module.cpp module.skel
#kded_kdetrayproxy_la_LDFLAGS = $(all_libraries) -module -avoid-version
#kded_kdetrayproxy_la_LIBADD = $(LIB_KDECORE) $(LIB_KDED)
#
#METASOURCES = AUTO
#
#noinst_HEADERS = kdetrayproxy.h module.h
#
#servicesdir = $(kde_servicesdir)/kded
#services_DATA = kdetrayproxy.desktop
