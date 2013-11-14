add_definitions(-DTRANSLATION_DOMAIN="kinit")

include(${CMAKE_CURRENT_SOURCE_DIR}/../KInitMacros.cmake)
include(ECMMarkNonGuiExecutable)

configure_file(config-kdeinit.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kdeinit.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(kdeinit)
add_subdirectory(kwrapper)
add_subdirectory(kioslave)
add_subdirectory(klauncher)

if (NOT WIN32)
  add_subdirectory(kshell)
  add_subdirectory(start_kdeinit)
endif ()
