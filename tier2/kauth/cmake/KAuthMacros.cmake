# Copyright (c) 2006-2009 Alexander Neundorf, <neundorf@kde.org>
# Copyright (c) 2006, 2007, Laurent Montel, <montel@kde.org>
# Copyright (c) 2007 Matthias Kretz <kretz@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# This macro adds the needed files for an helper executable meant to be used by applications using KAuth.
# It accepts the helper target, the helper ID (the DBUS name) and the user under which the helper will run on.
# This macro takes care of generate the needed files, and install them in the right location. This boils down
# to a DBus policy to let the helper register on the system bus, and a service file for letting the helper
# being automatically activated by the system bus.
# *WARNING* You have to install the helper in ${LIBEXEC_INSTALL_DIR} to make sure everything will work.
function(KAUTH_INSTALL_HELPER_FILES HELPER_TARGET HELPER_ID HELPER_USER)
    if(KAUTH_HELPER_BACKEND_NAME STREQUAL "DBUS")
        configure_file(${KAUTH_STUB_FILES_DIR}/dbus_policy.stub
                        ${CMAKE_CURRENT_BINARY_DIR}/${HELPER_ID}.conf)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${HELPER_ID}.conf
                DESTINATION ${SYSCONF_INSTALL_DIR}/dbus-1/system.d/)

        configure_file(${KAUTH_STUB_FILES_DIR}/dbus_service.stub
                        ${CMAKE_CURRENT_BINARY_DIR}/${HELPER_ID}.service)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${HELPER_ID}.service
                DESTINATION ${DBUS_SYSTEM_SERVICES_INSTALL_DIR})
    endif()
endfunction()

# This macro generates an action file, depending on the backend used, for applications using KAuth.
# It accepts the helper id (the DBUS name) and a file containing the actions (check kdelibs/kdecore/auth/example
# for file format). The macro will take care of generating the file according to the backend specified,
# and to install it in the right location. This (at the moment) means that on Linux (PolicyKit) a .policy
# file will be generated and installed into the policykit action directory (usually /usr/share/PolicyKit/policy/),
# and on Mac (Authorization Services) will be added to the system action registry using the native MacOS API during
# the install phase
function(KAUTH_INSTALL_ACTIONS HELPER_ID ACTIONS_FILE)

  if(KAUTH_BACKEND_NAME STREQUAL "APPLE")
    get_target_property(kauth_policy_gen KF5::kauth-policy-gen LOCATION)
    install(CODE "execute_process(COMMAND ${kauth_policy_gen} ${ACTIONS_FILE} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})")
  elseif(KAUTH_BACKEND_NAME STREQUAL "POLKITQT" OR KAUTH_BACKEND_NAME STREQUAL "POLKITQT-1")
    set(_output ${CMAKE_CURRENT_BINARY_DIR}/${HELPER_ID}.policy)
    get_filename_component(_input ${ACTIONS_FILE} ABSOLUTE)

    add_custom_command(OUTPUT ${_output}
                       COMMAND KF5::kauth-policy-gen ${_input} ${_output}
                       MAIN_DEPENDENCY ${_input}
                       WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                       COMMENT "Generating ${HELPER_ID}.policy"
                       DEPENDS KF5::kauth-policy-gen)
    add_custom_target("actions for ${HELPER_ID}" ALL DEPENDS ${_output})

    install(FILES ${_output} DESTINATION ${KAUTH_POLICY_FILES_INSTALL_DIR})
  endif()

endfunction()
