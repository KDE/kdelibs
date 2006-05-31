add_subdirectory( kfileaudiopreview )

add_definitions( ${QDBUS_DEFINITIONS} )

include_directories( ${KDE4_KPARTS_INCLUDES} )

########### next target ###############

set(kmediaplayer_LIB_SRCS
player.cpp
view.cpp
kmediaplayeradaptor.cpp
)

kde4_automoc(${kmediaplayer_LIB_SRCS})

kde4_add_library(kmediaplayer SHARED ${kmediaplayer_LIB_SRCS})

target_link_libraries(kmediaplayer  ${KDE4_KPARTS_LIBS} )

set_target_properties(kmediaplayer PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install_targets(${LIB_INSTALL_DIR} kmediaplayer )


########### install files ###############

install_files( ${SERVICETYPES_INSTALL_DIR} FILES kmediaplayerengine.desktop kmediaplayer.desktop )
install_files( /include/kmediaplayer FILES player.h view.h )
