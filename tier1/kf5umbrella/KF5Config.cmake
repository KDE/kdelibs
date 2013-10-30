
if (CMAKE_VERSION VERSION_LESS 2.8.9)
    message(FATAL_ERROR "KF5 requires at least CMake version 2.8.9")
endif()

if (NOT KF5_FIND_COMPONENTS)
    set(KF5_NOT_FOUND_MESSAGE "The KF5 package requires at least one component")
    set(KF5_FOUND False)
    return()
endif()

set(_KF5_FIND_PARTS_REQUIRED)
if (KF5_FIND_REQUIRED)
    set(_KF5_FIND_PARTS_REQUIRED REQUIRED)
endif()
set(_KF5_FIND_PARTS_QUIET)
if (KF5_FIND_QUIETLY)
    set(_KF5_FIND_PARTS_QUIET QUIET)
endif()

get_filename_component(_kf5_install_prefix "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

set(_KF5_NOTFOUND_MESSAGE)

foreach(module ${KF5_FIND_COMPONENTS})
    find_package(${module}
        ${_KF5_FIND_PARTS_QUIET}
        ${_KF5_FIND_PARTS_REQUIRED}
        PATHS "${_kf5_install_prefix}" NO_DEFAULT_PATH
    )
    if (NOT ${module}_FOUND)
        if (KF5_FIND_REQUIRED_${module})
            set(_KF5_NOTFOUND_MESSAGE "${_KF5_NOTFOUND_MESSAGE}Failed to find Qt5 component \"${module}\" config file at \"${_kf5_install_prefix}/${module}/${module}Config.cmake\"\n")
        elseif(NOT KF5_FIND_QUIETLY)
            message(WARNING "Failed to find KF5 component \"${module}\" config file at \"${_kf5_install_prefix}/${module}/${module}Config.cmake\"")
        endif()
    endif()
endforeach()

if (_KF5_NOTFOUND_MESSAGE)
    set(KF5_NOT_FOUND_MESSAGE "${_KF5_NOTFOUND_MESSAGE}")
    set(KF5_FOUND False)
endif()

macro(_kf5_check_file_exists)

endmacro()

_kf5_check_file_exists("${CMAKE_CURRENT_LIST_DIR}/KF5ConfigVersion.cmake")
