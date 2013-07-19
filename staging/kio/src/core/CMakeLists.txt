project(KIOCore)

include (ConfigureChecks.cmake)

configure_file(config-kiocore.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kiocore.h )

configure_file(config-kmountpoint.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kmountpoint.h)

set(kiocore_SRCS
  klocalsocket.cpp
  socketconnectionbackend.cpp
  connection.cpp
  directorysizejob.cpp
  forwardingslavebase.cpp
  chmodjob.cpp
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

  kio_ksslcertificatemanager.cpp # temporary (used by ktcpsocket)
  kio_ktcpsocket.cpp             # temporary (port to Qt API once it has hooks for users approving invalid certificates)
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
install(FILES
   org.kde.KDirNotify.xml
   org.kde.KPasswdServer.xml
   DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

add_library(KIOCore ${kiocore_SRCS})
generate_export_header(KIOCore)

target_include_directories(KIOCore PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/..>" # kio_version.h
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../include>" # kio/global.h
)

target_link_libraries(KIOCore
 ${KDE4_KDECORE_LIBS} # TODO remove once klocale is in a framework
 ${KConfig_CORE_LIBRARY}
 ${KCoreAddons_LIBRARIES}
 KI18n
 KService
 Qt5::Network
 Qt5::DBus
 Qt5::Concurrent
 Qt5::Xml   # davjob.cpp uses QDom
 ${kdeqt5staging_LIBRARIES} # Needed QUrlPathInfo
 ${KDBusAddons_LIBRARIES} # KDBusConnectionPool
)

if(ACL_FOUND)
  target_link_libraries(KIOCore ${ACL_LIBS})
endif(ACL_FOUND)

set_target_properties(KIOCore PROPERTIES VERSION ${ECM_VERSION_STRING}
                                          SOVERSION ${ECM_SOVERSION}
)

install(TARGETS KIOCore EXPORT kdelibsLibraryTargets ${ECM_TARGET_DEFAULT_ARGS})

install( FILES
  directorysizejob.h
  forwardingslavebase.h
  chmodjob.h
  deletejob.h
  copyjob.h
  filejob.h
  mkdirjob.h
  slavebase.h
  http_slave_defaults.h
  slaveconfig.h
  global.h
  http.h
  ioslave_defaults.h
  hostinfo.h
  jobtracker.h
  metadata.h
  udsentry.h
  jobuidelegateextension.h
  jobuidelegatefactory.h
  slaveinterface.h
  slave.h
  job.h
  scheduler.h
  jobclasses.h
  authinfo.h
  http.h
  davjob.h
  ${CMAKE_CURRENT_BINARY_DIR}/kiocore_export.h

   DESTINATION ${INCLUDE_INSTALL_DIR}/kio COMPONENT Devel)

install(FILES
  kacl.h
  kauthorized.h
  kcoredirlister.h
  kremoteencoding.h
  kdiskfreespaceinfo.h
  kdirnotify.h
  kfileitem.h
  kmountpoint.h
  knfsshare.h
  ksambashare.h
  ksambasharedata.h
  kprotocolinfo.h
  kprotocolmanager.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)
