
set(knotifications_SRCS
  knotification.cpp
  knotificationmanager.cpp
  kstatusnotifieritem.cpp
  kstatusnotifieritemdbus_p.cpp
  knotificationrestrictions.cpp
)

include_directories(
 ${DBusMenuQt5_INCLUDE_DIR}
)

find_package(DBusMenuQt5)
set_package_properties(DBusMenuQt5 PROPERTIES DESCRIPTION "DBusMenuQt"
                       URL "https://launchpad.net/libdbusmenu-qt" TYPE OPTIONAL
                       PURPOSE "Support for notification area menus via the DBusMenu protocol")
if (DBusMenuQt5_FOUND)
   set(HAVE_DBUSMENUQT 1)
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

find_package(X11)

if (X11_XTest_FOUND)
   set (HAVE_XTEST 1)
else ()
   set (HAVE_XTEST 0)
endif ()

configure_file(config-knotifications.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-knotifications.h )

add_library(KNotifications ${knotifications_SRCS})
generate_export_header(KNotifications)

target_link_libraries(KNotifications LINK_PUBLIC Qt5::Widgets)
target_link_libraries(KNotifications LINK_PRIVATE KWindowSystem Qt5::DBus)

if(HAVE_DBUSMENUQT)
  target_link_libraries(KNotifications ${DBusMenuQt5_LIBRARIES})
endif()

set_target_properties(KNotifications PROPERTIES VERSION   ${KNOTIFICATIONS_VERSION_STRING}
                                             SOVERSION ${KNOTIFICATIONS_SOVERSION}
)

install(TARGETS KNotifications EXPORT KNotificationsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  knotification.h
  kstatusnotifieritem.h
  knotificationrestrictions.h
  ${CMAKE_CURRENT_BINARY_DIR}/knotifications_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

install(FILES
    org.kde.StatusNotifierItem.xml
    org.kde.StatusNotifierWatcher.xml
    DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})
