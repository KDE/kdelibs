project(kiocore)

include (ConfigureChecks.cmake)

configure_file(config-kiocore.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kiocore.h )

configure_file(config-kmountpoint.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kmountpoint.h)

set(kiocore_SRCS
  klocalsocket.cpp
  socketconnectionbackend.cpp
  connection.cpp
  kdiskfreespaceinfo.cpp
  usernotificationhandler.cpp
  ksambasharedata.cpp
  ksambashare.cpp
  knfsshare.cpp
  kfileitem.cpp
  davjob.cpp
  deletejob.cpp
  copyjob.cpp
  filejob.cpp
  mkdirjob.cpp
  kpasswdserverloop.cpp
  kpasswdserver.cpp
  kremoteencoding.cpp
  sessiondata.cpp
  slavebase.cpp
  dataslave.cpp
  dataprotocol.cpp
  dataprotocol.cpp
  authinfo.cpp
  slaveinterface.cpp
  slave.cpp
  job_error.cpp
  job.cpp
  scheduler.cpp
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
  kmountpoint.cpp
  kcoredirlister.cpp
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


set_source_files_properties(org.kde.KPasswdServer.xml
        PROPERTIES INCLUDE authinfo.h
)
qt5_add_dbus_interface(kiocore_SRCS org.kde.KPasswdServer.xml kpasswdserver_interface)
install(FILES org.kde.KPasswdServer.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

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
 KService
 Qt5::Network
 Qt5::DBus
 Qt5::Concurrent
 Qt5::Xml   # davjob.cpp uses QDom
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
  kdiskfreespaceinfo.h
  ksambasharedata.h
  ksambashare.h
  knfsshare.h
  kfileitem.h
  davjob.h
  deletejob.h
  copyjob.h
  filejob.h
  mkdirjob.h
  kremoteencoding.h
  slavebase.h
  kacl.h
  kauthorized.h
  http_slave_defaults.h
  slaveconfig.h
  kprotocolmanager.h
  global.h
  http.h
  ioslave_defaults.h
  hostinfo.h
  jobtracker.h
  kprotocolinfo.h
  metadata.h
  udsentry.h
  jobuidelegateextension.h
  jobuidelegatefactory.h
  kmountpoint.h
  slaveinterface.h
  slave.h
  job.h
  scheduler.h
  jobclasses.h
  authinfo.h
  http.h
  kcoredirlister.h
  ${CMAKE_CURRENT_BINARY_DIR}/kiocore_export.h

   DESTINATION ${INCLUDE_INSTALL_DIR}/kio COMPONENT Devel)

install(FILES kdirnotify.h DESTINATION ${INCLUDE_INSTALL_DIR})
