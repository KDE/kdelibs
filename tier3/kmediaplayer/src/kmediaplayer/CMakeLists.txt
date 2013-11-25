set(kmediaplayer_LIB_SRCS
   player.cpp
   view.cpp
   kmediaplayeradaptor.cpp
)

add_library(KMediaPlayer ${kmediaplayer_LIB_SRCS})
generate_export_header(KMediaPlayer)

target_link_libraries(KMediaPlayer PUBLIC
   Qt5::Widgets
   KF5::KParts
   KF5::KI18n
)
target_link_libraries(KMediaPlayer PRIVATE
   KF5::XmlGui
)
set_target_properties(KMediaPlayer PROPERTIES
   VERSION ${KMEDIAPLAYER_VERSION_STRING}
   SOVERSION ${KMEDIAPLAYER_SOVERSION}
)

install(TARGETS KMediaPlayer
        EXPORT KMediaPlayerTargets
        ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
   player.h
   view.h
   ${CMAKE_CURRENT_BINARY_DIR}/kmediaplayer_export.h
   DESTINATION ${INCLUDE_INSTALL_DIR}/kmediaplayer
   COMPONENT Devel)
install(FILES
   kmediaplayerengine.desktop
   kmediaplayer.desktop
   DESTINATION ${SERVICETYPES_INSTALL_DIR})
install(FILES
   org.kde.KMediaPlayer.xml
   DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

