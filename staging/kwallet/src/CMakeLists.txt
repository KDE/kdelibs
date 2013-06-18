add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)

set(kwallet_SRCS
)

set(HAVE_KSECRETSSERVICE 0)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config-kwallet.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config-kwallet.h
)

option(MAC_USE_OSXKEYCHAIN "On OS X, use the keychain as backend for kwallet, instead of kwalletd.")

if (APPLE AND MAC_USE_OSXKEYCHAIN)
    FIND_LIBRARY(SECURITY_LIBRARY Security)
    set(kwallet_SRCS ${kwallet_LIB_SRCS} kwallet_mac.cpp)
else(APPLE AND MAC_USE_OSXKEYCHAIN)
     set(kwallet_SRCS ${kwallet_LIB_SRCS} kwallet.cpp)
endif(APPLE AND MAC_USE_OSXKEYCHAIN)

set(kwallet_xml org.kde.KWallet.xml)
install(FILES ${kwallet_xml} DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

qt5_add_dbus_interface( kwallet_SRCS ${kwallet_xml} kwallet_interface )

add_library(KWallet ${kwallet_SRCS})
generate_export_header(KWallet)

target_link_libraries(KWallet LINK_PUBLIC Qt5::Widgets)
target_link_libraries(KWallet LINK_PRIVATE Qt5::DBus KWindowSystem KConfigCore)

if(APPLE)
   target_link_libraries(KWallet ${CARBON_LIBRARY})
   if (MAC_USE_OSXKEYCHAIN)
     target_link_libraries(KWallet ${SECURITY_LIBRARY})
   endif (MAC_USE_OSXKEYCHAIN)
endif(APPLE)

set_target_properties(KWallet PROPERTIES VERSION   ${KWALLET_VERSION_STRING}
                                           SOVERSION ${KWALLET_SOVERSION}
)

install(TARGETS KWallet EXPORT KWalletTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  kwallet.h
  ${CMAKE_CURRENT_BINARY_DIR}/kwallet_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
