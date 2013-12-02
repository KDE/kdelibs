configure_file(config-kjobwidgets.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kjobwidgets.h )

set(kjobwidgets_SRCS
  kabstractwidgetjobtracker.cpp
  kdialogjobuidelegate.cpp
  kjobtrackerformatters.cpp
  kuiserverjobtracker.cpp
  kjobwidgets.cpp
  kstatusbarjobtracker.cpp
  kwidgetjobtracker.cpp
)

qt5_add_dbus_interfaces(kjobwidgets_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/org.kde.JobViewServer.xml )
qt5_add_dbus_interface(kjobwidgets_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/org.kde.JobView.xml jobviewiface )
qt5_add_dbus_interface(kjobwidgets_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/org.kde.JobViewV2.xml jobviewifacev2 )

add_library(KJobWidgets ${kjobwidgets_SRCS})

generate_export_header(KJobWidgets)
add_library(KF5::KJobWidgets ALIAS KJobWidgets)

target_link_libraries(KJobWidgets
   PUBLIC
      Qt5::Widgets
      KF5::CoreAddons    # KJob
   PRIVATE
      Qt5::DBus
      KF5::WidgetsAddons # KSqueezedTextLabel
)

if (HAVE_X11)
  find_package(Qt5X11Extras 5.2.0 REQUIRED NO_MODULE)
  target_link_libraries(KJobWidgets PRIVATE Qt5::X11Extras)
endif ()

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
  kabstractwidgetjobtracker.h
  kdialogjobuidelegate.h
  kuiserverjobtracker.h
  kjobwidgets.h
  kstatusbarjobtracker.h
  kwidgetjobtracker.h
  ${CMAKE_CURRENT_BINARY_DIR}/kjobwidgets_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
