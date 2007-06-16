
include_directories( ${CMAKE_SOURCE_DIR}/kdefx ${KDE4_KDECORE_INCLUDES} ${KDE4_KDEUI_INCLUDES} )

add_subdirectory( keramik )
if (OPENGL_FOUND AND NOT APPLE)
add_subdirectory( oxygen )
endif(OPENGL_FOUND AND NOT APPLE)
add_subdirectory( plastik )
add_subdirectory( riscos)



