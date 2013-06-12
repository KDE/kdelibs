add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)

set(kjobwidgets_SRCS
  kuiserverjobtracker.cpp
  kjobwidgets.cpp
)

qt5_add_dbus_interfaces(kjobwidgets_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/org.kde.JobViewServer.xml )
qt5_add_dbus_interface(kjobwidgets_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/org.kde.JobView.xml jobviewiface )
qt5_add_dbus_interface(kjobwidgets_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/org.kde.JobViewV2.xml jobviewifacev2 )

add_library(KJobWidgets ${kjobwidgets_SRCS})
generate_export_header(KJobWidgets)

target_link_libraries(KJobWidgets LINK_PUBLIC Qt5::Widgets Qt5::DBus KCoreAddons ki18n)

set_target_properties(KJobWidgets PROPERTIES VERSION   ${KJOBWIDGETS_VERSION_STRING}
                                             SOVERSION ${KJOBWIDGETS_SOVERSION}
)


install(TARGETS KJobWidgets EXPORT KJobWidgetsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
  org.kde.JobViewServer.xml
  org.kde.JobView.xml
  org.kde.JobViewV2.xml
  DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

install(FILES
  kuiserverjobtracker.h
  kjobwidgets.h
  ${CMAKE_CURRENT_BINARY_DIR}/kjobwidgets_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
