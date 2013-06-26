project(kdoctools)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

add_subdirectory(src)

configure_file(checkXML.in.cmake ${CMAKE_CURRENT_BINARY_DIR}/checkXML )
include(ConfigureChecks.cmake)
configure_file(config-kdoctools.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kdoctools.h )

