kde4_header()

add_subdirectory( kfileaudiopreview )

include_directories( ${KDE4_KPARTS_INCLUDES} )

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

target_link_libraries(kmediaplayer  ${KDE4_KPARTS_LIBS} )

set_target_properties(kmediaplayer PROPERTIES VERSION 0.0.0 SOVERSION 0 )
install_targets(${LIB_INSTALL_DIR} kmediaplayer )


########### install files ###############

install_files( ${SERVICETYPES_INSTALL_DIR} FILES kmediaplayerengine.desktop kmediaplayer.desktop )
install_files( /include/kmediaplayer FILES player.h playerdcopobject.h view.h )

kde4_footer()
