
find_package(Qt5 5.2.0 CONFIG REQUIRED Widgets DBus)

# This file handles all the logic for compiling KAuth's backends
include(ConfigureChecks.cmake)

# Configure a small file to tell BackendsManager what to use
configure_file(BackendsConfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/BackendsConfig.h)

set(libkauth_SRCS
    kauthaction.cpp
    kauthactionreply.cpp
    kauthexecutejob.cpp
    kauthobjectdecorator.cpp
    AuthBackend.cpp
    BackendsManager.cpp
    HelperProxy.cpp
    kauthhelpersupport.cpp
    backends/fake/FakeBackend.cpp
    backends/fakehelper/FakeHelperProxy.cpp
)

add_library(KF5Auth ${libkauth_SRCS})
generate_export_header(KF5Auth BASE_NAME KAuth)
add_library(KF5::Auth ALIAS KF5Auth)

target_link_libraries(KF5Auth PUBLIC Qt5::Core KF5::CoreAddons)
target_link_libraries(KF5Auth PRIVATE Qt5::Widgets Qt5::DBus)
set_target_properties(KF5Auth PROPERTIES VERSION   ${KAUTH_VERSION_STRING}
                                         SOVERSION ${KAUTH_SOVERSION}
)

# KAuth policy generator executable

# Compile only if fake backend has not been selected

if (NOT "${KAUTH_BACKEND_NAME}" STREQUAL "FAKE")
    # KAUTH_POLICY_GEN_SRCS has been generated from ConfigureChecks.cmake
    add_executable(kauth-policy-gen ${KAUTH_POLICY_GEN_SRCS})
    add_executable(KF5::kauth-policy-gen ALIAS kauth-policy-gen)

    # KAUTH_POLICY_GEN_LIBRARIES has been generated from ConfigureChecks.cmake
    target_link_libraries( kauth-policy-gen PRIVATE ${KAUTH_POLICY_GEN_LIBRARIES} )

    install( TARGETS kauth-policy-gen EXPORT KF5AuthTargets DESTINATION ${LIBEXEC_INSTALL_DIR})
endif ()

# KAuth backend plugin

if (NOT "${KAUTH_BACKEND_NAME}" STREQUAL "FAKE")
    set(KAUTH_BACKEND_SRCS ${KAUTH_BACKEND_SRCS})
    # KAuth::AuthBackend is not exported
    add_library(kauth_backend_plugin MODULE ${KAUTH_BACKEND_SRCS} AuthBackend.cpp)
    target_link_libraries(kauth_backend_plugin PRIVATE ${KAUTH_BACKEND_LIBS})
    set_target_properties(kauth_backend_plugin PROPERTIES PREFIX "")
    install(TARGETS kauth_backend_plugin
            LIBRARY DESTINATION ${KAUTH_BACKEND_PLUGIN_DIR}
            ARCHIVE DESTINATION ${KAUTH_BACKEND_PLUGIN_DIR}
            RUNTIME DESTINATION ${KAUTH_BACKEND_PLUGIN_DIR}
    )
endif ()

# KAuth helper plugin

if (NOT "${KAUTH_HELPER_BACKEND_NAME}" STREQUAL "FAKE")
    # KAuth::HelperProxy is not exported
    add_library(kauth_helper_plugin MODULE ${KAUTH_HELPER_BACKEND_SRCS} HelperProxy.cpp)
    target_link_libraries(kauth_helper_plugin PRIVATE ${KAUTH_HELPER_BACKEND_LIBS})
    set_target_properties(kauth_helper_plugin PROPERTIES PREFIX "")
    install(TARGETS kauth_helper_plugin
            LIBRARY DESTINATION ${KAUTH_HELPER_PLUGIN_DIR}
            ARCHIVE DESTINATION ${KAUTH_HELPER_PLUGIN_DIR}
            RUNTIME DESTINATION ${KAUTH_HELPER_PLUGIN_DIR}
    )
endif ()

install(TARGETS KF5Auth EXPORT KF5AuthTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    kauthaction.h
    kauthactionreply.h
    kauthexecutejob.h
    kauthhelpersupport.h
    kauthobjectdecorator.h
    kauth.h
    ${CMAKE_CURRENT_BINARY_DIR}/kauth_export.h
    DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

# TODO: Remove once we depend on polkit-qt1 > 0.99.0
# core/polkitqt1-actiondescription.h from polkit currently fails when built with -pedantic
string(REPLACE "--pedantic-errors" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(REPLACE "-pedantic-errors" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(REPLACE "--pedantic" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(REPLACE "-pedantic" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
