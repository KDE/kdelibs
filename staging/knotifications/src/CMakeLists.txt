add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)

set(knotifications_SRCS
  knotification.cpp
  knotificationmanager.cpp
)

qt5_add_dbus_interface(knotifications_SRCS org.kde.KNotify.xml knotify_interface)

add_library(KNotifications ${knotifications_SRCS})
generate_export_header(KNotifications)

target_link_libraries(KNotifications LINK_PUBLIC Qt5::Widgets)
target_link_libraries(KNotifications LINK_PRIVATE KWindowSystem Qt5::DBus)

set_target_properties(KNotifications PROPERTIES VERSION   ${KNOTIFICATIONS_VERSION_STRING}
                                             SOVERSION ${KNOTIFICATIONS_SOVERSION}
)

install(TARGETS KNotifications EXPORT KNotificationsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  knotification.h
  ${CMAKE_CURRENT_BINARY_DIR}/knotifications_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
