add_subdirectory( kfileaudiopreview )


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
install(TARGETS kmediaplayer  DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

install( FILES kmediaplayerengine.desktop kmediaplayer.desktop  DESTINATION  ${SERVICETYPES_INSTALL_DIR} )
install( FILES player.h view.h  DESTINATION  ${INCLUDE_INSTALL_DIR}/kmediaplayer )
