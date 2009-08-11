# - Try to find libkauth
# Once done this will define
#
#  KAUTH_FOUND - system has KAuth
#  KAUTH_INCLUDE_DIR - the KAuth include directory
#  KAUTH_LIBRARY - Link these to use all KAuth libs

include(FindPkgConfig)

if (UNIX AND NOT APPLE)
    find_package(PolkitQt REQUIRED)
    
    if (NOT KAUTH_FIND_QUIETLY)
        message(STATUS "Searching for Config::IniFiles perl module")
    endif (NOT KAUTH_FIND_QUIETLY)
    execute_process(COMMAND perl -e "use Config::IniFiles 2.52; 0;" OUTPUT_FILE /dev/null ERROR_FILE /dev/null RESULT_VARIABLE _result)
    if(_result)
        message(FATAL_ERROR "Could NOT find the perl module Config::IniFiles, version 2.52 or later")
    endif(_result)
endif (UNIX AND NOT APPLE)

find_path( KAUTH_INCLUDE_DIR kauth/kauth.h )
find_file( KAUTH_DBUS_POLICY_STUB share/kauth/dbus_policy.stub)
find_file( KAUTH_DBUS_SERVICE_STUB share/kauth/dbus_service.stub)

find_file(KAUTH_POLICY_GEN libexec/kauth-policy-gen)

find_library( KAUTH_LIBRARY NAMES kauth )

if (KAUTH_INCLUDE_DIR AND KAUTH_LIBRARY AND KAUTH_DBUS_POLICY_STUB AND KAUTH_DBUS_SERVICE_STUB AND KAUTH_POLICY_GEN)
    set(KAUTH_FOUND TRUE)
else()
    set(KAUTH_FOUND FALSE)
endif()

set(KAUTH_INCLUDE_DIR ${KAUTH_INCLUDE_DIR}/kauth )

if (KAUTH_FOUND)
    if (NOT KAUTH_FIND_QUIETLY)
        message(STATUS "Found KAuth: ${KAUTH_LIBRARY}")
    endif (NOT KAUTH_FIND_QUIETLY)
else (KAUTH_FOUND)
    if (KAUTH_FIND_REQUIRED)
        message(FATAL_ERROR "Could NOT find KAuth")
    endif (KAUTH_FIND_REQUIRED)
endif (KAUTH_FOUND)

macro(kde4_auth_add_helper _HELPER_TARGET _HELPER_ID _HELPER_USER)
    
    pkg_search_module( DBUS dbus-1 )

    set(HELPER_ID ${_HELPER_ID})
    set(HELPER_TARGET ${_HELPER_TARGET})
    set(HELPER_USER ${_HELPER_USER})
    
    add_executable(${HELPER_TARGET} ${ARGN})
    target_link_libraries(${HELPER_TARGET} ${KAUTH_LIBRARY} ${QT_QTCORE_LIBRARIES})
    install(TARGETS ${HELPER_TARGET} DESTINATION ${CMAKE_INSTALL_PREFIX}/libexec)
    
    configure_file(${KAUTH_DBUS_POLICY_STUB} ${CMAKE_CURRENT_BINARY_DIR}/${HELPER_ID}.conf)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${HELPER_ID}.conf DESTINATION /etc/dbus-1/system.d/)

    configure_file(${KAUTH_DBUS_SERVICE_STUB} ${CMAKE_CURRENT_BINARY_DIR}/${HELPER_ID}.service)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${HELPER_ID}.service DESTINATION ${DBUS_PREFIX}/share/dbus-1/system-services )
    

endmacro(kde4_auth_add_helper)

macro(kde4_auth_register_actions HELPER_ID ACTIONS_FILE)

if(APPLE)
    install(CODE "execute_process(COMMAND ${KAUTH_POLICY_GEN} ${ACTIONS_FILE} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})")
elseif(UNIX)
    set(_output ${CMAKE_CURRENT_BINARY_DIR}/${HELPER_ID}.policy)
    get_filename_component(_input ${ACTIONS_FILE} ABSOLUTE)
    
    add_custom_command(OUTPUT ${_output} 
                       COMMAND ${KAUTH_POLICY_GEN} ${_input} > ${_output} 
                       MAIN_DEPENDENCY ${_input}
                       WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                       COMMENT "Generating ${HELPER_ID}.policy")
    add_custom_target("actions for ${HELPER_ID}" ALL DEPENDS ${_output})

    install(FILES ${_output} DESTINATION ${POLICY_FILES_INSTALL_DIR})
endif()

endmacro(kde4_auth_register_actions)
