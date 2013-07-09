project(kiocore)

include (ConfigureChecks.cmake)

configure_file(config-kiocore.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kiocore.h )

set(kiocore_SRCS
  klocalsocket.cpp
  socketconnectionbackend.cpp
  connection.cpp
  slaveconfig.cpp
  kprotocolmanager.cpp
  hostinfo.cpp
  kdirnotify.cpp
  kauthorized.cpp
  kacl.cpp
  udsentry.cpp
  global.cpp
  metadata.cpp
  kprotocolinfo.cpp
  kprotocolinfofactory.cpp
  jobtracker.cpp
  jobuidelegateextension.cpp
  jobuidelegatefactory.cpp
)

if (UNIX)
   set(kiocore_SRCS ${kiocore_SRCS}
      klocalsocket_unix.cpp
   )
endif()
if (WIN32)
   set(kiocore_SRCS ${kiocore_SRCS}
      klocalsocket_win.cpp
   )
endif()

add_library(kiocore ${kiocore_SRCS})
generate_export_header(kiocore)

target_include_directories(kiocore PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/..>" # kio_version.h
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../include>" # kio/global.h
)

target_link_libraries(kiocore
 ${KDE4_KDECORE_LIBS} # TODO remove once klocale is in a framework
 ${KConfig_CORE_LIBRARY}
 ${KCoreAddons_LIBRARIES}
 ki18n
 kservice
 Qt5::Network
 Qt5::DBus
 Qt5::Concurrent
 ${kdeqt5staging_LIBRARIES} # Needed QUrlPathInfo
 ${KDBusAddons_LIBRARIES} # KDBusConnectionPool
)

if(ACL_FOUND)
  target_link_libraries(kiocore ${ACL_LIBS})
endif(ACL_FOUND)

set_target_properties(kiocore PROPERTIES VERSION ${ECM_VERSION_STRING}
                                          SOVERSION ${ECM_SOVERSION}
)

install(TARGETS kiocore EXPORT kdelibsLibraryTargets ${ECM_TARGET_DEFAULT_ARGS})

install( FILES
  kacl.h
  kauthorized.h
  http_slave_defaults.h
  slaveconfig.h
  kprotocolmanager.h
  global.h
  ioslave_defaults.h
  hostinfo.h
  jobtracker.h
  klocalsocket.h # TODO stop installing this once kiocore is done -- should not be public
  kprotocolinfo.h
  metadata.h
  udsentry.h
  jobuidelegateextension.h
  jobuidelegatefactory.h
  ${CMAKE_CURRENT_BINARY_DIR}/kiocore_export.h

   DESTINATION ${INCLUDE_INSTALL_DIR}/kio COMPONENT Devel)

install(FILES kdirnotify.h DESTINATION ${INCLUDE_INSTALL_DIR})
