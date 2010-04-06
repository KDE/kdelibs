####### checks for kdecore/kauth ###############

set(KDE4_AUTH_BACKEND_NAME "" CACHE STRING "Specifies the KAuth backend to build. Current available options are 
                                   PolkitQt, PolkitQt-1, Fake, Apple. Not setting this variable will build the most 
                                   appropriate backend for your system")

set(KAUTH_BACKEND ${KDE4_AUTH_BACKEND_NAME})

## Check if the user did not specify a backend to be built. If that is the case,
## we check what is the best backend to build on this system.
## 4.4: We leave polkit-0.9 having more priority over polkit-1. This will change from 4.5 on
if(NOT KAUTH_BACKEND)
    # Look for the most appropriate backend
    message(STATUS "No backend for KAuth was explicitly specified: probing system to find the best one available")
    if (APPLE)
        set (KAUTH_BACKEND "OSX")
    elseif (UNIX)
        macro_optional_find_package(PolkitQt)

        if (POLKITQT_FOUND)
            set (KAUTH_BACKEND "PolkitQt")
            macro_log_feature(POLKITQT_FOUND "PolkitQt" "Qt Wrapper around Policykit" "http://api.kde.org/polkit-qt"
                              FALSE "" "STRONGLY RECOMMENDED: Needed to make KAuth work (using PolicyKit < 0.90), hence to enable some workspace functionalities")
        else (POLKITQT_FOUND)
            macro_optional_find_package(PolkitQt-1)

            if (POLKITQT-1_FOUND)
                set (KAUTH_BACKEND "PolkitQt-1")
                macro_log_feature(POLKITQT-1_FOUND "PolkitQt-1" "Qt Wrapper around polkit-1" "http://techbase.kde.org/Polkit-Qt-1"
                                  FALSE "" "STRONGLY RECOMMENDED: Needed to make KAuth work (using polkit-1), hence to enable some workspace functionalities")
            else (POLKITQT-1_FOUND)
                # Nothing was found: notify and log the missing features
                macro_log_feature(POLKITQT_FOUND "PolkitQt" "Qt Wrapper around Policykit" "http://api.kde.org/polkit-qt"
                                  FALSE "" "STRONGLY RECOMMENDED: Needed to make KAuth work (using PolicyKit < 0.90), hence to enable some workspace functionalities")
                macro_log_feature(POLKITQT-1_FOUND "PolkitQt-1" "Qt Wrapper around polkit-1" "http://techbase.kde.org/Polkit-Qt-1"
                                  FALSE "" "STRONGLY RECOMMENDED: Needed to make KAuth work (using polkit-1), hence to enable some workspace functionalities")
                set (KAUTH_BACKEND "Fake")
            endif (POLKITQT-1_FOUND)
        endif (POLKITQT_FOUND)
    else(UNIX)
        set (KAUTH_BACKEND "Fake")
    endif(APPLE)

    # Case-insensitive
    string(TOUPPER ${KAUTH_BACKEND} KAUTH_BACKEND_UPPER)
    set (KAUTH_BACKEND ${KAUTH_BACKEND_UPPER})
else(NOT KAUTH_BACKEND)
    # Case-insensitive
    string(TOUPPER ${KAUTH_BACKEND} KAUTH_BACKEND_UPPER)
    set (KAUTH_BACKEND ${KAUTH_BACKEND_UPPER})

    # Check if the specified backend is valid. If it is not, we fall back to the Fake one
    if (NOT KAUTH_BACKEND STREQUAL "OSX" AND NOT KAUTH_BACKEND STREQUAL "POLKITQT" AND NOT KAUTH_BACKEND STREQUAL "POLKITQT-1" AND NOT KAUTH_BACKEND STREQUAL "FAKE")
        message ("WARNING: The KAuth Backend ${KAUTH_BACKEND} you specified does not exist. Falling back to Fake backend")
        set (KAUTH_BACKEND "FAKE")
    endif (NOT KAUTH_BACKEND STREQUAL "OSX" AND NOT KAUTH_BACKEND STREQUAL "POLKITQT" AND NOT KAUTH_BACKEND STREQUAL "POLKITQT-1" AND NOT KAUTH_BACKEND STREQUAL "FAKE")

    # Check requirements for each backend. If not, fall back to the fake one
    if (KAUTH_BACKEND STREQUAL "OSX" AND NOT APPLE)
        message ("WARNING: You chose the Apple KAuth backend but your system does not support it. Falling back to Fake backend")
        set (KAUTH_BACKEND "FAKE")
    endif (KAUTH_BACKEND STREQUAL "OSX" AND NOT APPLE)
    if (KAUTH_BACKEND STREQUAL "POLKITQT")
        macro_optional_find_package(PolkitQt)
        macro_log_feature(POLKITQT_FOUND "PolkitQt" "Qt Wrapper around Policykit" "http://api.kde.org/polkit-qt"
                          FALSE "" "STRONGLY RECOMMENDED: Needed to make KAuth work, hence to enable some workspace functionalities")

        if (NOT POLKITQT_FOUND)
            message ("WARNING: You chose the PolkitQt KAuth backend but you don't have PolkitQt installed.
                      Falling back to Fake backend")
            set (KAUTH_BACKEND "FAKE")
        endif (NOT POLKITQT_FOUND)
    endif (KAUTH_BACKEND STREQUAL "POLKITQT")
    if (KAUTH_BACKEND STREQUAL "POLKITQT-1")
        macro_optional_find_package(PolkitQt-1)
        macro_log_feature(POLKITQT-1_FOUND "PolkitQt-1" "Qt Wrapper around polkit-1" "http://techbase.kde.org/Polkit-Qt-1"
                          FALSE "" "STRONGLY RECOMMENDED: Needed to make KAuth work (using polkit-1), hence to enable some workspace functionalities")

        if (NOT POLKITQT-1_FOUND)
            message ("WARNING: You chose the PolkitQt-1 KAuth backend but you don't have PolkitQt-1 installed.
                      Falling back to Fake backend")
            set (KAUTH_BACKEND "FAKE")
        endif (NOT POLKITQT-1_FOUND)
    endif (KAUTH_BACKEND STREQUAL "POLKITQT-1")
endif(NOT KAUTH_BACKEND)

set(KDE4_AUTH_BACKEND_NAME ${KAUTH_BACKEND} CACHE STRING "Specifies the KAuth backend to build. Current available options are 
                                   PolkitQt, PolkitQt-1, Fake, Apple. Not setting this variable will build the most 
                                   appropriate backend for your system" FORCE)

# Add the correct libraries depending on the backend, and eventually set the policy files install location
if(KDE4_AUTH_BACKEND_NAME STREQUAL "OSX")
    set (KAUTH_COMPILING_OSX_BACKEND TRUE)

    find_library(CORE_FOUNDATION_LIBRARY CoreFoundation)
    find_library(SECURITY_LIBRARY Security)

    message(STATUS "Building Apple KAuth backend")

    set(KAUTH_BACKEND_SRCS
        auth/backends/mac/AuthServicesBackend.cpp
    )

    set(KAUTH_BACKEND_LIBS ${SECURITY_LIBRARY} ${QT_QTCORE_LIBRARY})
elseif(KDE4_AUTH_BACKEND_NAME STREQUAL "POLKITQT")
    set (KAUTH_COMPILING_POLKITQT_BACKEND TRUE)

    message(STATUS "Building PolkitQt KAuth backend")

    include_directories(${POLKITQT_INCLUDE_DIR})

    set(KAUTH_BACKEND_SRCS
       auth/backends/policykit/PolicyKitBackend.cpp
    )

    set(KAUTH_BACKEND_LIBS ${POLKITQT_CORE_LIBRARY} ${QT_QTCORE_LIBRARY})

    set(KDE4_AUTH_POLICY_FILES_INSTALL_DIR ${POLKITQT_POLICY_FILES_INSTALL_DIR} CACHE STRING 
        "Where policy files generated by KAuth will be installed" FORCE)
elseif(KDE4_AUTH_BACKEND_NAME STREQUAL "POLKITQT-1")
    set (KAUTH_COMPILING_POLKITQT1_BACKEND TRUE)

    message(STATUS "Building PolkitQt-1 KAuth backend")

    include_directories(${POLKITQT-1_INCLUDE_DIR})

    set(KAUTH_BACKEND_SRCS
        auth/backends/polkit-1/Polkit1Backend.cpp
    )

    set(KAUTH_BACKEND_LIBS ${POLKITQT-1_CORE_LIBRARY} ${QT_QTCORE_LIBRARY})

    set(KDE4_AUTH_POLICY_FILES_INSTALL_DIR ${POLKITQT-1_POLICY_FILES_INSTALL_DIR} CACHE STRING 
        "Where policy files generated by KAuth will be installed" FORCE)
elseif(KDE4_AUTH_BACKEND_NAME STREQUAL "FAKE")
    set (KAUTH_COMPILING_FAKE_BACKEND TRUE)

    message(STATUS "Building Fake KAuth backend")
    message("WARNING: No valid KAuth backends will be built. The library will not work properly unless compiled with
             a working backend")
endif()

# KAuth policy generator executable source probing
set(KAUTH_POLICY_GEN_SRCS
    auth/policy-gen/policy-gen.cpp )
set(KAUTH_POLICY_GEN_LIBRARIES ${QT_QTCORE_LIBRARY})

if(KDE4_AUTH_BACKEND_NAME STREQUAL "OSX")
   set(KAUTH_POLICY_GEN_SRCS ${KAUTH_POLICY_GEN_SRCS}
       auth/backends/mac/kauth-policy-gen-mac.cpp)
   set(KAUTH_POLICY_GEN_LIBRARIES ${KAUTH_POLICY_GEN_LIBRARIES} ${CORE_FOUNDATION_LIBRARY} ${SECURITY_LIBRARY})
elseif(KDE4_AUTH_BACKEND_NAME STREQUAL "POLKITQT")
   set(KAUTH_POLICY_GEN_SRCS ${KAUTH_POLICY_GEN_SRCS}
       auth/backends/policykit/kauth-policy-gen-polkit.cpp )
elseif(KDE4_AUTH_BACKEND_NAME STREQUAL "POLKITQT-1")
  set(KAUTH_POLICY_GEN_SRCS ${KAUTH_POLICY_GEN_SRCS}
      auth/backends/polkit-1/kauth-policy-gen-polkit1.cpp )
endif()

########################
# Helper backend probing

set(KDE4_AUTH_HELPER_BACKEND_NAME "" CACHE STRING "Specifies the KAuth helper backend to build. Current available options are
                                   DBus, Fake. Not setting this variable will build the most appropriate backend for your system")

set(KAUTH_HELPER_BACKEND ${KDE4_AUTH_HELPER_BACKEND_NAME})

if(NOT KAUTH_HELPER_BACKEND)
    # No checks needed, just set the dbus backend
    set(KAUTH_HELPER_BACKEND "DBus")
    string(TOUPPER ${KAUTH_HELPER_BACKEND} KAUTH_HELPER_BACKEND_UPPER)
    set (KAUTH_HELPER_BACKEND ${KAUTH_HELPER_BACKEND_UPPER})
else(NOT KAUTH_HELPER_BACKEND)
    # No checks needed here either
    string(TOUPPER ${KAUTH_HELPER_BACKEND} KAUTH_HELPER_BACKEND_UPPER)
    set (KAUTH_HELPER_BACKEND ${KAUTH_HELPER_BACKEND_UPPER})
endif(NOT KAUTH_HELPER_BACKEND)

set(KDE4_AUTH_HELPER_BACKEND_NAME ${KAUTH_HELPER_BACKEND} CACHE STRING "Specifies the KAuth helper backend to build. Current
                                                            available options are DBus, Fake. Not setting this variable will
                                                            build the most appropriate backend for your system" FORCE)

# Add the correct libraries/files depending on the backend
if(KDE4_AUTH_HELPER_BACKEND_NAME STREQUAL "DBUS")
    set (KAUTH_COMPILING_DBUS_HELPER_BACKEND TRUE)

    qt4_add_dbus_adaptor(kauth_dbus_adaptor_SRCS
                        auth/backends/dbus/org.kde.auth.xml
                        auth/backends/dbus/DBusHelperProxy.h
                        KAuth::DBusHelperProxy)

    set(KAUTH_HELPER_BACKEND_SRCS
        auth/backends/dbus/DBusHelperProxy.cpp
        ${kauth_dbus_adaptor_SRCS}
    )

    set(KAUTH_HELPER_BACKEND_LIBS kdecore)

    # Install some files as well
    install( FILES auth/backends/dbus/org.kde.auth.conf
             DESTINATION ${SYSCONF_INSTALL_DIR}/dbus-1/system.d )

    install( FILES auth/backends/dbus/dbus_policy.stub
                   auth/backends/dbus/dbus_service.stub
             DESTINATION ${DATA_INSTALL_DIR}/kauth COMPONENT Devel)
elseif(KDE4_AUTH_HELPER_BACKEND_NAME STREQUAL "FAKE")
    set (KAUTH_COMPILING_FAKE_HELPER_BACKEND TRUE)

    message("WARNING: No valid KAuth helper backends will be built. The library will not work properly unless compiled with
             a working backend")
endif()


# Set directories for plugins
if(NOT WIN32)
_set_fancy(KAUTH_HELPER_PLUGIN_DIR "${PLUGIN_INSTALL_DIR}/plugins/kauth/helper" "Where KAuth's helper plugin will be installed")
_set_fancy(KAUTH_BACKEND_PLUGIN_DIR "${PLUGIN_INSTALL_DIR}/plugins/kauth/backend" "Where KAuth's backend plugin will be installed")
#set(KAUTH_OTHER_PLUGIN_DIR "${QT_PLUGINS_DIR}/kauth/plugins")
else(NOT WIN32)
set(KAUTH_HELPER_PLUGIN_DIR "${PLUGIN_INSTALL_DIR}/plugins/kauth/helper")
set(KAUTH_BACKEND_PLUGIN_DIR "${PLUGIN_INSTALL_DIR}/plugins/kauth/backend")
endif(NOT WIN32)

## End
