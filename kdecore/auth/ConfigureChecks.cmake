####### checks for kdecore/kauth ###############

set(KAUTH_BACKEND "" CACHE STRING "Specifies the KAuth backend to build. Current available options are 
                                   PolkitQt, Fake, Apple. Not setting this variable will build the most 
                                   appropriate backend for your system")
## Check if the user did not specify a backend to be built. If that is the case,
## we check what is the best backend to build on this system.
## 4.4: We leave polkit-0.9 having more priority over polkit-1. This will change from 4.5 on
if(NOT KAUTH_BACKEND)
    if (APPLE)
        set (KAUTH_BACKEND "OSX")
    elseif (UNIX)
        macro_optional_find_package(PolkitQt)
        macro_log_feature(POLKITQT_FOUND "PolkitQt" "Qt Wrapper around Policykit" "http://api.kde.org/polkit-qt"
                          FALSE "" "STRONGLY RECOMMENDED: Needed to make KAuth work, hence to enable some workspace functionalities")

        if (POLKITQT_FOUND)
            set (KAUTH_BACKEND "PolkitQt")
        else (POLKITQT_FOUND)
            macro_optional_find_package(PolkitQt1)
            macro_log_feature(POLKITQT1_FOUND "PolkitQt1" "Qt Wrapper around polkit-1" "http://techbase.kde.org/Polkit-Qt-1"
                          FALSE "" "Using PolkitQt1")# TODO: some comment
            if (POLKITQT1_FOUND)
                set (KAUTH_BACKEND "PolkitQt1")
            else (POLKITQT1_FOUND)
                set (KAUTH_BACKEND "Fake")
            endif (POLKITQT1_FOUND)
        endif (POLKITQT_FOUND)
    else(UNIX)
        set (KAUTH_BACKEND "Fake")
    endif(APPLE)

    # Case-insensitive
    string(TOUPPER ${KAUTH_BACKEND} KAUTH_BACKEND)
else(NOT KAUTH_BACKEND)
    # Case-insensitive
    string(TOUPPER ${KAUTH_BACKEND} KAUTH_BACKEND)
    
    # Check if the specified backend is valid. If it is not, we fall back to the Fake one
    if (NOT KAUTH_BACKEND STREQUAL "OSX" AND NOT KAUTH_BACKEND STREQUAL "POLKITQT" AND NOT KAUTH_BACKEND STREQUAL "POLKITQT1" AND NOT KAUTH_BACKEND STREQUAL "FAKE")
        message ("WARNING: The KAuth Backend ${KAUTH_BACKEND} you specified does not exist. Falling back to Fake backend")
        set (KAUTH_BACKEND "Fake")
    endif (NOT KAUTH_BACKEND STREQUAL "OSX" AND NOT KAUTH_BACKEND STREQUAL "POLKITQT" AND NOT KAUTH_BACKEND STREQUAL "POLKITQT1" AND NOT KAUTH_BACKEND STREQUAL "FAKE")

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
    if (KAUTH_BACKEND STREQUAL "POLKITQT1")
        macro_optional_find_package(PolkitQt1)
        macro_log_feature(POLKITQT1_FOUND "PolkitQt1" "Qt Wrapper around polkit-1" "http://techbase.kde.org/Polkit-Qt-1"
                          FALSE "" "Using PolkitQt1")# TODO: some comment

        if (NOT POLKITQT1_FOUND)
            message ("WARNING: You chose the PolkitQt-1 KAuth backend but you don't have PolkitQt-1 installed.
                      Falling back to Fake backend")
            set (KAUTH_BACKEND "FAKE")
        endif (NOT POLKITQT1_FOUND)
    endif (KAUTH_BACKEND STREQUAL "POLKITQT1")
endif(NOT KAUTH_BACKEND)

# Add the correct libraries depending on the backend
if(KAUTH_BACKEND STREQUAL "OSX")
    find_library(CORE_FOUNDATION_LIBRARY CoreFoundation)
    find_library(SECURITY_LIBRARY Security)
elseif(KAUTH_BACKEND STREQUAL "POLKITQT")
    include_directories(${POLKITQT_INCLUDE_DIR})
elseif(KAUTH_BACKEND STREQUAL "POLKITQT1")
    include_directories(${POLKITQT1_INCLUDE_DIR})
endif()
