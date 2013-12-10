
set(knotifications_SRCS
  knotification.cpp
  knotificationmanager.cpp
  kpassivepopup.cpp
  kstatusnotifieritem.cpp
  kstatusnotifieritemdbus_p.cpp
  knotificationrestrictions.cpp
)

find_package(DBusMenuQt5)
set_package_properties(DBusMenuQt5 PROPERTIES DESCRIPTION "DBusMenuQt"
                       URL "https://launchpad.net/libdbusmenu-qt" TYPE OPTIONAL
                       PURPOSE "Support for notification area menus via the DBusMenu protocol")
if (DBusMenuQt5_FOUND)
   set(HAVE_DBUSMENUQT 1)
   include_directories(${DBusMenuQt5_INCLUDE_DIRS})
else()
   set(HAVE_DBUSMENUQT 0)
endif()

qt5_add_dbus_interface(knotifications_SRCS org.kde.KNotify.xml knotify_interface)


qt5_add_dbus_adaptor(knotifications_SRCS     org.kde.StatusNotifierItem.xml
                     kstatusnotifieritemdbus_p.h KStatusNotifierItemDBus)


set(statusnotifierwatcher_xml org.kde.StatusNotifierWatcher.xml)
qt5_add_dbus_interface(knotifications_SRCS ${statusnotifierwatcher_xml} statusnotifierwatcher_interface)
set(notifications_xml org.freedesktop.Notifications.xml)
qt5_add_dbus_interface(knotifications_SRCS ${notifications_xml} notifications_interface)

configure_file(config-knotifications.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-knotifications.h )

add_library(KF5Notifications ${knotifications_SRCS})
generate_export_header(KF5Notifications BASE_NAME KNotifications)
add_library(KF5::Notifications ALIAS KF5Notifications)

target_link_libraries(KF5Notifications PUBLIC
   Qt5::Widgets
)
target_link_libraries(KF5Notifications PRIVATE
   KF5::WindowSystem
   Qt5::DBus
)

if(X11_FOUND)
   target_link_libraries(KF5Notifications PRIVATE ${X11_X11_LIB} Qt5::X11Extras)
endif()

if(X11_XTest_FOUND)
  target_link_libraries(KF5Notifications PRIVATE ${X11_XTest_LIB})
endif()

if(HAVE_DBUSMENUQT)
  target_link_libraries(KF5Notifications PRIVATE ${DBusMenuQt5_LIBRARIES})
endif()

set_target_properties(KF5Notifications PROPERTIES VERSION   ${KNOTIFICATIONS_VERSION_STRING}
                                                  SOVERSION ${KNOTIFICATIONS_SOVERSION}
                                                  EXPORT_NAME Notifications
)

install(TARGETS KF5Notifications EXPORT KF5NotificationsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  knotification.h
  kpassivepopup.h
  kstatusnotifieritem.h
  knotificationrestrictions.h
  ${CMAKE_CURRENT_BINARY_DIR}/knotifications_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

install(FILES
    org.kde.StatusNotifierItem.xml
    org.kde.StatusNotifierWatcher.xml
    DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})
