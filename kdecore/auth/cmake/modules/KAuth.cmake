if(APPLE)
    find_library(CORE_FOUNDATION_LIBRARY CoreFoundation)
    find_library(SECURITY_LIBRARY Security)
    set(KAUTH_DEPENDENCIES ${CORE_FOUNDATION_LIBRARY} ${SECURITY_LIBRARY} ${QT_QTCORE_LIBRARIES})
elseif(UNIX)
    find_package(PolkitQt)
    set(KAUTH_DEPENDENCIES ${POLKITQT_CORE_LIBRARY} ${QT_QTCORE_LIBRARIES})
endif()

macro(kde4_auth_register_helper _HELPER_ID _HELPER_TARGET _HELPER_USER )

    set(HELPER_ID ${_HELPER_ID})
    set(HELPER_TARGET ${_HELPER_TARGET})
    set(HELPER_USER ${_HELPER_USER})

    configure_file(${CMAKE_INSTALL_PREFIX}/share/kauth/dbus_policy.stub ${CMAKE_CURRENT_BINARY_DIR}/${HELPER_ID}.conf)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${HELPER_ID}.conf DESTINATION /etc/dbus-1/system.d/)

    configure_file(${CMAKE_INSTALL_PREFIX}/share/kauth/dbus_service.stub ${CMAKE_CURRENT_BINARY_DIR}/${HELPER_ID}.service)
    dbus_add_activation_system_service(${CMAKE_CURRENT_BINARY_DIR}/${HELPER_ID}.service)
    install(TARGETS ${HELPER_TARGET} DESTINATION ${CMAKE_INSTALL_PREFIX}/libexec)

endmacro(kde4_auth_register_helper)


macro(kde4_auth_register_actions HELPER_ID ACTIONS_FILE)

    execute_process(COMMAND ${CMAKE_INSTALL_PREFIX}/libexec/kauth-policy-gen-polkit.pl ${ACTIONS_FILE} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} OUTPUT_FILE ${CMAKE_CURRENT_BINARY_DIR}/${HELPER_ID}.policy)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${HELPER_ID}.policy DESTINATION ${POLICY_FILES_INSTALL_DIR})

endmacro(kde4_auth_register_actions)