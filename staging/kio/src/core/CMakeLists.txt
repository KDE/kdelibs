project(kiocore)

include (ConfigureChecks.cmake)

configure_file(config-kiocore.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kiocore.h )


include_directories(
  ${QT_INCLUDES}
  ${kcoreaddons_SOURCE_DIR}/src/jobs
  ${kcoreaddons_BINARY_DIR}/src
  ${kconfig_SOURCE_DIR}/src/core
  ${kconfig_BINARY_DIR}/src/core
  ${CMAKE_CURRENT_BINARY_DIR}
)

include_directories( ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} )

set(kiocore_SRCS
  klocalsocket.cpp
  socketconnectionbackend.cpp
  connection.cpp
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
 ${KDE4_KDECORE_LIBS} # TODO remove once i18n is in a framework
 kconfigcore
 kcoreaddons
 ${QT_QTCORE_LIBRARY}
 ${QT_QTNETWORK_LIBRARY}
 ${QT_QTDBUS_LIBRARY}
)

set_target_properties(kiocore PROPERTIES VERSION ${ECM_VERSION_STRING}
                                          SOVERSION ${ECM_SOVERSION}
)

install(TARGETS kiocore EXPORT kdelibsLibraryTargets ${ECM_TARGET_DEFAULT_ARGS})

install( FILES
  kprotocolinfo.h
  jobtracker.h

   ${CMAKE_CURRENT_BINARY_DIR}/kiocore_export.h
   DESTINATION ${INCLUDE_INSTALL_DIR}/kio COMPONENT Devel)

