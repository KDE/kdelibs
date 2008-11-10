add_subdirectory( kfileaudiopreview )


include_directories( ${KDE4_KPARTS_INCLUDES} )

########### next target ###############

set(kmediaplayer_LIB_SRCS
player.cpp
view.cpp
kmediaplayeradaptor.cpp
)


kde4_add_library(kmediaplayer SHARED ${kmediaplayer_LIB_SRCS})

target_link_libraries(kmediaplayer  ${KDE4_KPARTS_LIBS} )
target_link_libraries(kmediaplayer  LINK_INTERFACE_LIBRARIES ${KDE4_KPARTS_LIBS} ${QT_QTGUI_LIBRARY} )

set_target_properties(kmediaplayer PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
)
install(TARGETS kmediaplayer EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

install( FILES kmediaplayer_export.h player.h view.h  DESTINATION  ${INCLUDE_INSTALL_DIR}/kmediaplayer  COMPONENT Devel)

install( FILES kmediaplayerengine.desktop kmediaplayer.desktop  DESTINATION  ${SERVICETYPES_INSTALL_DIR} )
install( FILES org.kde.KMediaPlayer.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )
