set(kmediaplayer_LIB_SRCS
   player.cpp
   view.cpp
   kmediaplayeradaptor.cpp
)

add_library(KF5MediaPlayer ${kmediaplayer_LIB_SRCS})
generate_export_header(KF5MediaPlayer BASE_NAME KMediaPlayer)
add_library(KF5::MediaPlayer ALIAS KF5MediaPlayer)

target_link_libraries(KF5MediaPlayer PUBLIC
   Qt5::Widgets
   KF5::Parts
   KF5::I18n
)
target_link_libraries(KF5MediaPlayer PRIVATE
   KF5::XmlGui
)
set_target_properties(KF5MediaPlayer PROPERTIES
   VERSION ${KMEDIAPLAYER_VERSION_STRING}
   SOVERSION ${KMEDIAPLAYER_SOVERSION}
   EXPORT_NAME MediaPlayer
)

install(TARGETS KF5MediaPlayer
        EXPORT KF5MediaPlayerTargets
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

