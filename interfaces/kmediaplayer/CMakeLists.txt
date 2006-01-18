kde4_header()

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(kfileaudiopreview_subdir)")
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kmediaplayer_LIB_SRCS
player.cpp
view.cpp
)

kde4_automoc(${kmediaplayer_LIB_SRCS})

set( kmediaplayer_DCOP_SKEL_SRCS
playerdcopobject.h
)

kde4_add_dcop_skels(kmediaplayer_LIB_SRCS ${kmediaplayer_DCOP_SKEL_SRCS})

kde4_add_library(kmediaplayer SHARED ${kmediaplayer_LIB_SRCS})

target_link_libraries(kmediaplayer  ${QT_AND_KDECORE_LIBS} kparts )

set_target_properties(kmediaplayer PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(/lib kmediaplayer )


########### install files ###############

install_files( ${KDE4_SERVICESTYPES_DIR} FILES kmediaplayerengine.desktop kmediaplayer.desktop )

kde4_footer()



#original Makefile.am contents follow:

#if include_ARTS
#kfileaudiopreview_subdir = kfileaudiopreview
#endif
#
#SUBDIRS = . $(kfileaudiopreview_subdir)
#INCLUDES = -I$(srcdir)/.. $(all_includes)
#
#lib_LTLIBRARIES = libkmediaplayer.la
#
#libkmediaplayer_la_SOURCES = player.cpp playerdcopobject.skel view.cpp
#libkmediaplayer_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -no-undefined
#libkmediaplayer_la_LIBADD = $(LIB_KPARTS)
#libkmediaplayer_la_METASOURCES = AUTO
#
#kmediaplayerinclude_HEADERS = player.h playerdcopobject.h view.h
#kmediaplayerincludedir = $(includedir)/kmediaplayer
#
#kde_servicetypes_DATA = kmediaplayerengine.desktop kmediaplayer.desktop
