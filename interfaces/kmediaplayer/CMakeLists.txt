add_subdirectory( kfileaudiopreview )

find_package(Phonon4Qt5 4.6.60 REQUIRED NO_MODULE)

########### next target ###############

set(kmediaplayer_LIB_SRCS
player.cpp
view.cpp
kmediaplayeradaptor.cpp
)


kde4_add_library(kmediaplayer ${LIBRARY_TYPE} ${kmediaplayer_LIB_SRCS})
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/kmediaplayer)
generate_export_header(kmediaplayer EXPORT_FILE_NAME kmediaplayer/kmediaplayer_export.h)

target_link_libraries(kmediaplayer  ${KDE4_KPARTS_LIBS} XmlGui)
target_link_libraries(kmediaplayer  LINK_INTERFACE_LIBRARIES ${KDE4_KPARTS_LIBS} Qt5::Widgets KI18n)

set_target_properties(kmediaplayer PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
)
install(TARGETS kmediaplayer EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

install( FILES ${CMAKE_CURRENT_BINARY_DIR}/kmediaplayer/kmediaplayer_export.h player.h view.h  DESTINATION  ${INCLUDE_INSTALL_DIR}/kmediaplayer  COMPONENT Devel)

install( FILES kmediaplayerengine.desktop kmediaplayer.desktop  DESTINATION  ${SERVICETYPES_INSTALL_DIR} )
install( FILES org.kde.KMediaPlayer.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )
