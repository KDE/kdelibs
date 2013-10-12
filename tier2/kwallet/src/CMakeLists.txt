
set(kwallet_SRCS
)

set(HAVE_KSECRETSSERVICE 0)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config-kwallet.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config-kwallet.h
)

if(APPLE)
    option(MAC_USE_OSXKEYCHAIN "On OS X, use the keychain as backend for kwallet, instead of kwalletd.")
else()
    set(MAC_USE_OSXKEYCHAIN FALSE)
endif()

if (MAC_USE_OSXKEYCHAIN)
    FIND_LIBRARY(SECURITY_LIBRARY Security)
    set(kwallet_SRCS ${kwallet_LIB_SRCS} kwallet_mac.cpp)
else()
    set(kwallet_SRCS ${kwallet_LIB_SRCS} kwallet.cpp)
    set(kwallet_xml org.kde.KWallet.xml)
    qt5_add_dbus_interface( kwallet_SRCS ${kwallet_xml} kwallet_interface )
endif()

add_library(KWallet ${kwallet_SRCS})
generate_export_header(KWallet)
add_library(KF5::KWallet ALIAS KWallet)

target_link_libraries(KWallet LINK_PUBLIC  Qt5::Widgets
                              LINK_PRIVATE KWindowSystem # KWindowSystem::allowExternalProcessWindowActivation
                                           KF5::KConfigCore   # used to store the wallet to be used
)

if(MAC_USE_OSXKEYCHAIN)
   target_link_libraries(KWallet LINK_PRIVATE ${CARBON_LIBRARY}
                                              ${SECURITY_LIBRARY})
elseif(APPLE)
    target_link_libraries(KWallet LINK_PRIVATE ${CARBON_LIBRARY})
else()
    target_link_libraries(KWallet LINK_PRIVATE Qt5::DBus)
    install(FILES ${kwallet_xml} DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})
endif()

set_target_properties(KWallet PROPERTIES VERSION   ${KWALLET_VERSION_STRING}
                                         SOVERSION ${KWALLET_SOVERSION}
)

install(TARGETS KWallet EXPORT KWalletTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
  kwallet.h
  ${CMAKE_CURRENT_BINARY_DIR}/kwallet_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
