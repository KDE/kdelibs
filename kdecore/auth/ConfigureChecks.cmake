####### checks for kdecore/kauth ###############

if(APPLE)
    find_library(CORE_FOUNDATION_LIBRARY CoreFoundation)
    find_library(SECURITY_LIBRARY Security)
elseif(UNIX)
    if(NOT POLKITQT_FOUND)
        message("WARNING: PolkitQt library not found. KAuth will be built with a fake backend")
    endif(NOT POLKITQT_FOUND)
    if (POLKITQT_INCLUDE_DIR)
      include_directories(${POLKITQT_INCLUDE_DIR})
    endif()
endif()
