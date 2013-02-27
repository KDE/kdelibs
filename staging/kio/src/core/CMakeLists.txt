project(kiocore)

include (ConfigureChecks.cmake)

configure_file(config-kiocore.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kiocore.h )


include_directories(
  ${QT_INCLUDES}
  ${KCoreAddons_INCLUDE_DIRS}
  ${KConfig_INCLUDE_DIRS}
  ${ki18n_SOURCE_DIR}/src
  ${ki18n_BINARY_DIR}/src
  ${kiocore_BINARY_DIR}/..           #kio_version.h
  ${kservice_SOURCE_DIR}/src/plugin  #kpluginfactory.h
  ${kservice_SOURCE_DIR}/src/services
  ${kservice_SOURCE_DIR}/src/sycoca  #ksycocaentry.h
  ${kservice_BINARY_DIR}/src
  ${CMAKE_CURRENT_BINARY_DIR}
)

include_directories( ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} )

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

target_link_libraries(kiocore
 ${KDE4_KDECORE_LIBS} # TODO remove once klocale is in a framework
 ${KConfig_CORE_LIBRARY}
 ${KCoreAddons_LIBRARIES}
 ki18n
 kservice
 ${QT_QTNETWORK_LIBRARY}
 ${QT_QTDBUS_LIBRARY}
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
  klocalsocket.h
  kprotocolinfo.h
  metadata.h
  udsentry.h

   ${CMAKE_CURRENT_BINARY_DIR}/kiocore_export.h
   DESTINATION ${INCLUDE_INSTALL_DIR}/kio COMPONENT Devel)

