project(kactivities)

include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
   ${KDE4_KIO_INCLUDES}
   )

set(
   kactivities_LIB_SRCS

   kactivityconsumer.cpp
   kactivitycontroller.cpp
   kactivityinfo.cpp
   kactivitymanager_p.cpp
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
   kactivityconsumer.h
   kactivitycontroller.h
   kactivityinfo.h
   )

install(
   FILES ${kactivities_LIB_HEADERS}
   DESTINATION ${INCLUDE_INSTALL_DIR}/
   COMPONENT Devel
   )

install(
   TARGETS kactivities
   EXPORT kdelibsLibraryTargets
   ${INSTALL_TARGETS_DEFAULT_ARGS}
   )

