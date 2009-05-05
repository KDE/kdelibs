
# Project needs a name of course
project(notification)

# Find the required Libaries
find_package(KDE4 REQUIRED)
include(KDE4Defaults)

add_subdirectory(cmake)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
   )

set(knotificationitem_SRCS knotificationitem.cpp knotificationitemdbus_p.cpp)
set(knotificationitemtest_SRC test/knotificationitemtest.cpp)

QT4_ADD_DBUS_ADAPTOR(knotificationitem_SRCS     org.kde.NotificationItem.xml
                     knotificationitemdbus_p.h Experimental::KNotificationItemDBus)

set(systemtraydaemon_xml org.kde.NotificationItemWatcher.xml)
QT4_ADD_DBUS_INTERFACE(knotificationitem_SRCS ${systemtraydaemon_xml} notificationwatcher_interface)

set(visualnotifications_xml org.kde.VisualNotifications.xml)
QT4_ADD_DBUS_INTERFACE(knotificationitem_SRCS ${visualnotifications_xml} visualnotifications_interface)

kde4_add_library(knotificationitem-1 SHARED ${knotificationitem_SRCS})
target_link_libraries(knotificationitem-1 ${KDE4_KDEUI_LIBS})

kde4_add_executable(knotificationitemtest ${knotificationitemtest_SRC})
target_link_libraries(knotificationitemtest ${KDE4_KDEUI_LIBS} knotificationitem-1)


install(TARGETS knotificationitem-1 EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    knotificationitem.h
    knotificationitem_export.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/knotificationitem-1 COMPONENT Devel)

install(FILES
    org.kde.NotificationItem.xml
    org.kde.NotificationItemWatcher.xml
    DESTINATION ${KDE4_DBUS_INTERFACES_DIR})
