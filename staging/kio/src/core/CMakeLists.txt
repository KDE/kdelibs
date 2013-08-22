project(KIOCore)

include (ConfigureChecks.cmake)

configure_file(config-kiocore.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kiocore.h )

configure_file(config-kmountpoint.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kmountpoint.h)

find_package(OpenSSL)
set_package_properties(OpenSSL PROPERTIES DESCRIPTION "Support for secure network communications (SSL and TLS)"
                       URL "http://openssl.org"
                       TYPE RECOMMENDED
                       PURPOSE "KDE uses OpenSSL for the bulk of secure communications, including secure web browsing via HTTPS"
                      )
if(OPENSSL_FOUND)
   set(KSSL_HAVE_SSL 1)
   include_directories(${OPENSSL_INCLUDE_DIR})
endif(OPENSSL_FOUND)

configure_file(kssl/ksslconfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/ksslconfig.h )

set(kiocore_SRCS
  klocalsocket.cpp
  socketconnectionbackend.cpp
  connection.cpp
  krecentdocument.cpp
  kfileitemlistproperties.cpp
  kemailsettings.cpp
  tcpslavebase.cpp
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
  kssl/ksslsettings.cpp          # temporary (used by tcpslavebase)
  kssl/kopenssl.cpp              # temporary (used by ksslsettings)
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

qt5_add_dbus_interface(kiocore_SRCS org.kde.KLauncher.xml klauncher_interface)

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
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/kssl>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../include>" # kio/global.h
)

target_link_libraries(KIOCore
LINK_PUBLIC
 ${KCoreAddons_LIBRARIES}   # KJob
 KService
LINK_PRIVATE
 KI18n
 Qt5::Network
 Qt5::Concurrent            # QtConcurrentRun in hostinfo.cpp
 ${KConfig_CORE_LIBRARY}
 Qt5::Xml                   # davjob.cpp uses QDom
 ${KDBusAddons_LIBRARIES}   # KDBusConnectionPool
)

if(ACL_FOUND)
  target_link_libraries(KIOCore LINK_PRIVATE ${ACL_LIBS})
endif(ACL_FOUND)

set_target_properties(KIOCore PROPERTIES VERSION ${KIO_VERSION_STRING}
                                       SOVERSION ${KIO_SOVERSION}
)

install(TARGETS KIOCore EXPORT KIOTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
  tcpslavebase.h
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
  davjob.h
  ${CMAKE_CURRENT_BINARY_DIR}/kiocore_export.h

   DESTINATION ${INCLUDE_INSTALL_DIR}/kio COMPONENT Devel)

install(FILES
  kacl.h
  kauthorized.h
  kcoredirlister.h
  kemailsettings.h
  kremoteencoding.h
  kdiskfreespaceinfo.h
  kdirnotify.h
  kfileitem.h
  kfileitemlistproperties.h
  kmountpoint.h
  knfsshare.h
  ksambashare.h
  ksambasharedata.h
  kprotocolinfo.h
  kprotocolmanager.h
  krecentdocument.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)
