project(kactivities)

include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
   ${KDE4_KIO_INCLUDES}
   )

set(
   kactivities_LIB_SRCS

   consumer.cpp
   controller.cpp
   info.cpp
   manager_p.cpp
   resourceinstance.cpp
   )

qt4_add_dbus_interface(
   kactivities_LIB_SRCS

   org.kde.ActivityManager.xml
   activitymanager_interface
)


kde4_add_library(
   kactivities SHARED
   ${kactivities_LIB_SRCS}
   )

set_target_properties(
   kactivities
   PROPERTIES
   VERSION ${KDE_NON_GENERIC_LIB_VERSION}
   SOVERSION ${KDE_NON_GENERIC_LIB_SOVERSION}
   )

target_link_libraries(
   kactivities
   ${KDE4_KDECORE_LIBS}
   )

## install

set(
   kactivities_LIB_HEADERS
   consumer.h
   controller.h
   info.h
   resourceinstance.h
   )

set(
   kactivities_LIB_PRETTY_HEADERS
   includes/Activities/Consumer
   includes/Activities/Controller
   includes/Activities/Info
   includes/Activities/ResourceInstance
   )

install(
   FILES ${kactivities_LIB_HEADERS}
   DESTINATION ${INCLUDE_INSTALL_DIR}/kactivities
   COMPONENT Devel
   )

install(
   FILES ${kactivities_LIB_PRETTY_HEADERS}
   DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/Activities
   COMPONENT Devel
   )

install(
   TARGETS kactivities
   EXPORT kdelibsLibraryTargets
   ${INSTALL_TARGETS_DEFAULT_ARGS}
   )

