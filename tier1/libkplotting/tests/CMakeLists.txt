set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
set(kplottest_SRCS testplot_widget.cpp testplot_main.cpp)
kde4_add_executable(kplottest TEST ${kplottest_SRCS})
target_link_libraries(kplottest kplotting)
