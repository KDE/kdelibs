####### checks for kdecore/kauth ###############

if(APPLE)
    find_library(CORE_FOUNDATION_LIBRARY CoreFoundation)
    find_library(SECURITY_LIBRARY Security)
elseif(UNIX)
    find_package(PolkitQt REQUIRED)
    if(NOT POLKITQT_FOUND)
        message(FATAL_ERROR "KDE Requires the PolkitQt library")
    endif(NOT POLKITQT_FOUND)
    include_directories(${POLKITQT_INCLUDE_DIR})
endif()
