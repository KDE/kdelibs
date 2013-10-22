configure_file(config-kde4attic.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kde4attic.h )

set(kde4attic_SRCS
  kgesture.cpp
  kgesturemap.cpp
  ksslcertificatemanager.cpp
  ktcpsocket.cpp
)

add_library(KDE4Attic ${kde4attic_SRCS})
generate_export_header(KDE4Attic)
add_library(KF5::KDE4Attic ALIAS KDE4Attic)

target_link_libraries(KDE4Attic
  LINK_PUBLIC
    Qt5::PrintSupport
    Qt5::Network
    KCompletion
    XmlGui
  LINK_PRIVATE
    Qt5::Concurrent
    KF5::KI18n
    KUnitConversion
    KNotifications
    KF5::KWindowSystem
    KF5::KGuiAddons
)

set_target_properties(KDE4Attic PROPERTIES VERSION   ${KDE4ATTIC_VERSION_STRING}
                                           SOVERSION ${KDE4ATTIC_SOVERSION}
)

install(TARGETS KDE4Attic EXPORT KDE4AtticTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  kgesture.h
  ktcpsocket.h

  ${CMAKE_CURRENT_BINARY_DIR}/kde4attic_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

