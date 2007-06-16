
include_directories( ${CMAKE_SOURCE_DIR}/kdefx ${KDE4_KDECORE_INCLUDES} )

add_subdirectory( keramik )
if (NOT APPLE)
add_subdirectory( oxygen )
endif(NOT APPLE)
add_subdirectory( plastik )
add_subdirectory( riscos)



